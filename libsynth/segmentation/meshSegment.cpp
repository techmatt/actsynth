
#include "libsynth.h"

//#include <mlibCGAL.h>

void MeshSegment::init()
{
    points.resize(vertexIndices.size());
    const auto& V = mesh->getVertices();
    for (size_t i = 0; i < points.size(); i++)
        points[i] = V[vertexIndices[i]].position;

    aabb.reset();
    for (const vec3f &pt : points)
        aabb.include(pt);

    oobb = computeUpOrientedOBB(points);
}

OBBf MeshSegment::computeUpOrientedOBB(const vector<vec3f> &points)
{
    /*vector<vec2f> points2D(points.size());
    for (size_t i = 0; i < points.size(); i++)
    {
        const vec3f &p = points[i];
        points2D[i] = vec2f(p.x, p.y);
    }
        
#ifdef _DEBUG
    if (points2D.size() > 20)
        points2D.resize(20);
#endif

    auto PCA = ml::math::pointSetPCA(points2D);*/

    cout << "Mesh segment not currently used!" << endl;
    return OBBf();
    //return ml::CGALWrapperf::computeOrientedBoundingBox(points, ml::CGALWrapperf::CONSTRAIN_Z);
    

	//return OBBf(points, vec3f(PCA[0].first, 0.0f), vec3f(PCA[1].first, 0.0f), vec3f::eZ);
    //return OBBf(&points[0], points.size(), vec3f::eX, vec3f::eY, vec3f::eZ);
}
