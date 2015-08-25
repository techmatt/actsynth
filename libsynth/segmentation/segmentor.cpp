
#include "libsynth.h"

#include "./disjoint-set.h"
#include "./segment-graph.h"

vector<MeshSegment> Segmentor::makeFilteredSegments(const TriMeshf& mesh) const
{
    const size_t minPoints = 100;
    const float minDiagonalLength = 0.05f;

    const SegmentFilter segFilter = [&](const MeshSegment& s) {
        return s.points.size() >= minPoints && s.oobb.getDiagonalLength() >= minDiagonalLength;
    };

    auto vertexSegIndices = segment(mesh);
    vector<MeshSegment*> segmentPtrs;
    createSegments(mesh, vertexSegIndices, minPoints, segFilter, segmentPtrs);

    vector<MeshSegment> result;
    for (MeshSegment *seg : segmentPtrs)
    {
        result.push_back(*seg);
        delete seg;
    }
    cout << "post-filter segments: " << result.size() << endl;
    return result;
}

MeshEdges Segmentor::getEdges(const TriMeshf& mesh)
{
    MeshEdges edges;
    const vector<ml::vec3ui>& indices = mesh.getIndices();
    for (int i = 0; i < mesh.getIndices().size(); i++)
    {
        UINT v1 = indices[i].x;
        UINT v2 = indices[i].y;
        UINT v3 = indices[i].z;
        edges.push_back(std::make_pair(v1, v2));
        edges.push_back(std::make_pair(v1, v3));
        edges.push_back(std::make_pair(v3, v2));
    }
    return edges;
}

void Segmentor::createSegments(const TriMeshf& mesh, const vector<UINT>& ids, UINT minPoints, const SegmentFilter& filter, vector<MeshSegment*> &acceptedSegs)
{
    cout << "creating segments?" << endl;
    const size_t nVertices = ids.size();
    std::unordered_map<size_t, vector<UINT>> map;

    for (UINT i = 0; i < nVertices; i++)
        map[ids[i]].push_back(i);

    acceptedSegs.clear();

    UINT iSeg = 0;
    
    for (const auto &g : map)
    {
        if (g.second.size() >= minPoints)
        {
            MeshSegment *newSegment = new MeshSegment(mesh, g.second, iSeg++);
            if (filter(*newSegment))
                acceptedSegs.push_back(newSegment);
            else
                delete newSegment;
        }
    }
}

vector<UINT> SegmentorFelzenswalb::segment(const TriMeshf& mesh) const
{
    size_t N = mesh.getVertices().size();
    cout << "Segmenting mesh with " << N << " vertices...";
    
    const MeshEdges& es = getEdges(mesh);
    size_t Ne = es.size();
    edge* edges = new edge[Ne];
    const auto& verts = mesh.getVertices();

    for (size_t i = 0; i < Ne; i++)
    {
        int a = es[i].first;
        int b = es[i].second;

        edges[i].a = a;
        edges[i].b = b;

        const vec3f& n1 = verts[a].normal;
        const vec3f& n2 = verts[b].normal;
        const vec3f& p1 = verts[a].position;
        const vec3f& p2 = verts[b].position;

        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;
        float dz = p2.z - p1.z;
        float dd = sqrt(dx * dx + dy * dy + dz * dz);
        dx /= dd; dy /= dd; dz /= dd;

        float dot = vec3f::dot(n1, n2);
        float dot2 = n2.x * dx + n2.y * dy + n2.z * dz;
        float ww = 1.0f - dot;
        if (dot2 > 0) { ww = ww * ww; } // make it much less of a problem if convex regions have normal difference
        if (colorWeight > 0.0f)
        {
            const vec4f& c1 = verts[a].color;
            const vec4f& c2 = verts[b].color;
            //const float d = vec3f::dist(c1, c2);
            //ww = (1 - m_colorWeight) * ww +  m_colorWeight * d;
            const vec4f& c1hsl = ml::ColorUtils::rgbToHsl<vec4f>(c1);
            const vec4f& c2hsl = ml::ColorUtils::rgbToHsl<vec4f>(c2);
            //const vec4f& c1rgb = ml::ColorUtils::hslToRgb<vec4f>(c1hsl);
            //const vec4f& c2rgb = ml::ColorUtils::hslToRgb<vec4f>(c2hsl);
            // Find difference in H and L, handling wrapping and make range [0,1]
            const float hdiff = 2.0f * std::min(fabsf(c1hsl[0] - c2hsl[0]), fabsf(c2hsl[0] - c1hsl[0] + 1));
            const float ldiff = 2.0f * std::min(fabsf(c1hsl[2] - c2hsl[2]), fabsf(c2hsl[2] - c1hsl[2] + 1));
            const float hldiff = sqrtf(hdiff*hdiff + ldiff*ldiff);
            ww = (1.0f - colorWeight) * ww + colorWeight * hldiff;
        }

        edges[i].w = ww;
    }
    cout << "graph done... ";

    // Segment!
    universe* u = segment_graph(static_cast<int>(N), static_cast<int>(Ne), edges, threshold);
    cout << u->num_sets() << " initial segs. ";

    // Join small segments
    for (int j = 0; j < Ne; j++)
    {
        int a = u->find(edges[j].a);
        int b = u->find(edges[j].b);
        if ((a != b) && ((u->size(a) < minSegmentVertices) || (u->size(b) < minSegmentVertices))) {
            u->join(a, b);
        }
    }

    cout << " Done: " << u->num_sets() << " final segs" << endl;
    // Return segment indices
    vector<UINT> out(N);
    for (int q = 0; q < N; q++)
    {
        out[q] = u->find(q);
    }
    delete u;
    return out;
}
