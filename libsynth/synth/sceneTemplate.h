
struct Plane
{
    bbox3f convexHullBBox() const
    {
        return bbox3f(convex_hull);
    }
    OBBf oriented() const
    {
        return bbox3f(convex_hull);
    }
    bool isObjectSupportPlane() const
    {
        const float minSupportSurfaceArea = 1.0f;
        const float minZHeight = 0.7f;
        const float maxZHeight = 1.5f;

        const bbox3f bbox = convexHullBBox();
        
        const bool normalCheck = fabs(normal.z) > 0.9f;
        const bool surfaceAreaCheck = (bbox.getExtentX() * bbox.getExtentY() > minSupportSurfaceArea);
        const bool heightCheck = (bbox.getCenter().z >= minZHeight) && (bbox.getCenter().z <= maxZHeight);

        return (normalCheck && surfaceAreaCheck && heightCheck);
    }
    bool isAgentSupportPlane() const
    {
        const float minSupportSurfaceArea = 0.2f * 0.2f;
        const float maxSupportSurfaceArea = 0.5f * 0.5f;
        const float minZHeight = 0.2f;
        const float maxZHeight = 0.8f;

        const bbox3f bbox = convexHullBBox();
        const float surfaceArea = bbox.getExtentX() * bbox.getExtentY();

        const bool normalCheck = fabs(normal.z) > 0.9f;
        const bool surfaceAreaCheck = (surfaceArea > minSupportSurfaceArea) && (surfaceArea < maxSupportSurfaceArea);
        const bool heightCheck = (bbox.getCenter().z >= minZHeight) && (bbox.getCenter().z <= maxZHeight);

        return (normalCheck && surfaceAreaCheck && heightCheck);
    }

    void computeSimplifiedHull();
    void transform(const mat4f &m);

    int id;
    ml::vec3f normal;
    ml::vec3f centroid;
    ml::vec3f bbox_corner;
    ml::vec3f bbox_width;
    ml::vec3f bbox_height;
    std::vector<ml::vec3f> concave_hull;
    std::vector<ml::vec3f> convex_hull;
    std::vector<int> point_indices;

    std::vector<ml::vec3f> simplified_hull; //used for agent orientations
};

class GeometricRepresentation
{
public:
    void init(const Scan& s);

    // score a 3D scene for how much its geometry deviates from the scanned geometric representation
    double scoreScene(const Scene& scene) const;

    // compute the incremental change in score for inserting the given object into the scene.
    // ideally this would be scoreScene(scene + object) - scoreScene(scene), but this is not strictly necessary
    double scoreObjectPlacement(const Scene& scene, const DisembodiedObject &object, const mat4f &modelToWorld) const;

    std::vector<Plane> planes;
    ColumnRepresentation columns;

private:
    void transform(const mat4f &m);

    bool load(const std::string& filename);
    void loadVersion1(std::ifstream& fin);
};

namespace sg {
    namespace core {
        struct SceneGrokMap;
    }
    //namespace vis {
    //    struct HeatMapEntry;
    //}
}
class ScenegrokDistribution
{
public:
    // requires the virtual scene prior; uses the scan to construct the 'agent map' sampling (a scan needs to a have pointer to cached scenegrok output)
    void init(const Database &_database, const Scan &_scan, const GeometricRepresentation &_geometry);

    // generate a discrete agent set from the continuous 'agent map' distribution
    vector<Agent> generateAgentSamples(const std::string& verb) const;
    vector<Agent> generateAgentSamples() const;

    // accepts v if it is on top of a valid plane in geometry
    bool acceptPoint(const vec2f &v) const;
    //double heatMapEntryScore(const sg::vis::HeatMapEntry &e) const;

    bool hasSceneGrokMap(const string &activityName) const {
        return (sceneGrokMapMap.count(activityName) != 0);
    }

    const sg::core::SceneGrokMap& getSceneGrokMap(const string &activityName) const {
        if (sceneGrokMapMap.count(activityName) == 0) std::cerr << "activity not found " + activityName << std::endl;
        return *sceneGrokMapMap.at(activityName);
    }
private:
    map<string, sg::core::SceneGrokMap*> sceneGrokMapMap;
    const Database *database;
    const Scan *scan;
    const GeometricRepresentation *geometry;
};

struct SceneTemplate
{
    void init(const Database& database, const Scan &s);

    ScenegrokDistribution agentDistribution;
    GeometricRepresentation geometry;
    
    //
    // the scan itself should NOT be referenced -- the goal is to "compress" the scan into only its relevant components.
    // but for debugging and visualization purposes it can still be useful to have the original scan available.
    //
    const Scan *debugScan;
};
