
#include "libsynth.h"

void ScenegrokDistribution::init(const Database &_database, const Scan &_scan, const GeometricRepresentation &_geometry)
{
    database = &_database;
    scan = &_scan;
    geometry = &_geometry;

    auto testAgents = generateAgentSamples();
}

bool ScenegrokDistribution::acceptPoint(const vec2f &v) const
{
    const float minSupportSurfaceArea = 0.2f * 0.2f;
    const float minZHeight = 0.2f;
    const float maxZHeight = 0.8f;
    const float bboxScaleFactor = 0.5f;

    for (const Plane &plane : geometry->planes)
    {
        bbox3f bbox = plane.convexHullBBox();
        
        vec3f vHeight(v, bbox.getCenter().z);
        bbox.scale(bboxScaleFactor);
        const bool insideCheck = bbox.intersects(vHeight);

        if (plane.isAgentSupportPlane() && insideCheck)
        {
            return true;
        }
    }

    return false;
}

vector<Agent> ScenegrokDistribution::generateAgentSamples() const
{
    return scan->agents;
}

void GeometricRepresentation::init(const Scan& s)
{
    load(s.pathName + ".abs");
    transform(s.transform);
    cout << "loaded " << s.name + ".abs with " << planes.size() << " planes" << endl;

    columns = ColumnRepresentation::constructFromScan(s, synthParams().columnDimension, *this);
}

void Plane::computeSimplifiedHull()
{
    if (simplified_hull.size() > 0) return;

    simplified_hull = convex_hull;

    while (true) {
        if (simplified_hull.size() <= 3) break;

        float bestScore = 0.0f;
        size_t bestMap = -1;
        for (size_t i = 0; i < simplified_hull.size(); i++) {
            const ml::vec3f& p0 = simplified_hull[i];
            const ml::vec3f& p1 = simplified_hull[(i + 1) % simplified_hull.size()];
            const ml::vec3f& p2 = simplified_hull[(i + 2) % simplified_hull.size()];

            float d = std::abs((p0 - p1).getNormalized() | (p1 - p2).getNormalized());

            if (d > bestScore) {
                bestScore = d;
                bestMap = (i + 1)%simplified_hull.size();
            }
        }

        if (bestScore > 0.95) {
            simplified_hull.erase(simplified_hull.begin() + bestMap);
        }
        else {
            break;
        }
    }
}


void Plane::transform(const mat4f &m)
{
    for (vec3f &v : convex_hull)
        v = m * v;
    computeSimplifiedHull();
}

void GeometricRepresentation::transform(const mat4f &m)
{
    for (Plane &p : planes)
        p.transform(m);
}

double GeometricRepresentation::scoreScene(const Scene& scene) const {
    return 0.0;
}

double GeometricRepresentation::scoreObjectPlacement(const Scene& scene, const DisembodiedObject &object, const mat4f &modelToWorld) const {
    return 0.0;
}

void SceneTemplate::init(const Database& database, const Scan &s) {
    debugScan = &s;
    geometry.init(s);
    agentDistribution.init(database, s, geometry);
}

static void loadVec3(std::ifstream& fin, ml::vec3f& vec) {
    float x, y, z;
    fin.read((char*)(&x), sizeof(float));
    fin.read((char*)(&y), sizeof(float));
    fin.read((char*)(&z), sizeof(float));
    vec = ml::vec3f(x, y, z);
    return;
}

static void loadVec3Array(std::ifstream& fin, std::vector<ml::vec3f>& array) {
    int array_size;
    fin.read((char*)(&array_size), sizeof(int));
    array.resize(array_size);
    for (int i = 0; i < array_size; ++i) {
        loadVec3(fin, array[i]);
    }
    return;
}

static void loadIntArray(std::ifstream& fin, std::vector<int>& array) {
    int array_size;
    fin.read((char*)(&array_size), sizeof(int));
    array.resize(array_size);
    for (int i = 0; i < array_size; ++i) {
        fin.read((char*)(&(array[i])), sizeof(int));
    }
    return;
}

bool GeometricRepresentation::load(const std::string& filename) {
    std::ifstream fin(filename, std::ios::binary);
    if (!fin.good())
        return false;

    int version = 0;
    fin.read((char*)(&version), sizeof(int));

    if (version == 1) {
        loadVersion1(fin);
        return true;
    }

    return false;
}

void GeometricRepresentation::loadVersion1(std::ifstream& fin) {
    int plane_num = (int)planes.size();
    fin.read((char*)(&plane_num), sizeof(int));

    // these numbers are for future use
    int place_holder = 0;
    for (int i = 0; i < 15; ++i)
        fin.read((char*)(&place_holder), sizeof(int));

    planes.resize(plane_num);
    for (size_t i = 0, i_end = planes.size(); i < i_end; ++i) {
        Plane& plane = planes[i];
        fin.read((char*)(&(plane.id)), sizeof(int));
        loadVec3(fin, plane.normal);
        loadVec3(fin, plane.centroid);
        loadVec3(fin, plane.bbox_corner);
        loadVec3(fin, plane.bbox_width);
        loadVec3(fin, plane.bbox_height);
        loadVec3Array(fin, plane.concave_hull);
        loadVec3Array(fin, plane.convex_hull);
        loadIntArray(fin, plane.point_indices);
        for (int i = 0; i < 4; ++i)
            fin.read((char*)(&place_holder), sizeof(int));
    }

    return;
}
