
enum SynthesizerStage
{
    SynthesizerStagePlaceAgents,
    SynthesizerStageChooseCandidateObjects,
    SynthesizerStageEvaluatePlacementA,
    SynthesizerStageEvaluatePlacementB,
    SynthesizerStagePlaceBestObject,
};

string stageName(SynthesizerStage stage);

namespace SynthUtil
{
    void makeBBoxTopTriangles(const OBBf &obb, Trianglef &t0, Trianglef &t1);
    Polygonf makeOBBTopPolygon(const OBBf &obb);
};

struct ActivityBlocker
{
    OBBf obb;
    TriMeshAcceleratorBVHf *accel;
};

class SynthScore
{
public:
    static double collisionScale()
    {
        return 2000.0;
    }
    static double overhangScale()
    {
        return 20.0;
    }
    static double baseOffsetScale()
    {
        return 4.0;
    }
    static double handOrientationScale()
    {
        return 2.0;
    }
    static double scanObjectScale()
    {
        return 20.0;
    }
    static double gazeScale()
    {
        return 1.0;
    }
    static double fingertipScale()
    {
        return 1.0;
    }
    static double backSupportScale()
    {
        return 0.4;
    }
    static double wallDistScale()
    {
        return 0.0;
    }
    static double wallRotationScale()
    {
        return 0.0;
    }

    static double wallDistScore(const Category &c, float wallDist);
    static int wallDistAcceptable(const Category &c, float wallDist, float threshold);

    static double distributionScore(const Distribution3D *distribution, const vec3f &offset, float scaleFactor);
    static int distributionPointAcceptable(const Distribution3D *distribution, const vec3f &offset, float threshold);

    static double handOrientationScore(const DisembodiedObject &object, const mat4f &modelToWorld, const Agent &agent);

    static double overhangScore(double overhang);

    static double scanLocatorResultsScore(const ScanLocatorResults &results, const DisembodiedObject &object, const mat4f &modelToWorld);
    static double scanLocatorResultsScore(const ScanLocatorResults &results, const DisembodiedObject &object, const mat4f &modelToWorld, float distThreshold);
	static double scanObjectScore(const ScanLocalizedObject &scanObject, const OBBf &oobb, float distThreshold);

    static double gazeClamp(const double gazeScore)
    {
        if (gazeScore >= 0.93f)
            return 1.0f;
        return gazeScore;
    }
};

//
// SynthesizerAgent is extra per-agent information beyond what is contained in the corresponding Agent object in the scene
//

struct AgentCategoryData
{
    AgentCategoryData()
    {
        category = NULL;
        activityInsertionScore = -1.0;
    }
    const Category *category;
    UINT sceneCount;
    UINT sampledCount;

    //
    // probability that this agent-activity should include an additional instance of this category
    //
    double activityInsertionScore;
};

struct SynthesizerAgent
{
    Agent *agent;

    vector<AgentCategoryData> makeSortedCategories() const;

    map<string, AgentCategoryData> categories;
};

struct InteractionRayEntry
{
    Rayf ray;
    vec3f centroidPos;

    bool visible;

    //
    // score is how good this ray is. a good ray is typically "head on" (monitor directly facing agent better than at an oblique angle)
    // and satisfies other constraints (a backSupport ray is behind the agent.)
    //
    double score;
};

struct InteractionRays
{
    double score(bool visibilityOptional) const;
    double score(bool visibilityOptional, const ml::TriMeshRayAcceleratorf &occluder, const mat4f &occluderTransform) const;

    vector<InteractionRayEntry> rays;
};

struct ObjectScoreAgentEntry
{
    ObjectScoreAgentEntry()
    {
        baseOffsetScore = 0.0f;
        handOrientationScore = 0.0f;
    }

    vector< pair<const InteractionRays*, string> > allInteractions() const
    {
        vector< pair<const InteractionRays*, string> > result;
        result.push_back(std::make_pair(&fingertip, "fingertip"));
        result.push_back(std::make_pair(&gaze, "gaze"));
        result.push_back(std::make_pair(&backSupport, "backSupport"));
        return result;
    }

    UINT agentIndex;

    vec3f baseOffset;
    double baseOffsetScore;

    double handOrientationScore;

    InteractionRays fingertip;
    InteractionRays gaze;
    InteractionRays backSupport;
};

struct ObjectScoreEntry
{
    ObjectScoreEntry()
    {

    }
    ObjectScoreEntry(float _rotation, const vector<UINT> &agentIndices)
    {
        rotation = _rotation;
        collisionScore = 0.0f;
        collisionBBoxScore = 0.0f;
        wallDistScore = 0.0f;
        wallRotationScore = 0.0f;
        scanPlacementScore = 0.0f;
        acceptable = 0.0f;
        leftRightScore = 0.0f;
        agents.resize(agentIndices.size());
        for (UINT agentIndex = 0; agentIndex < agentIndices.size(); agentIndex++)
            agents[agentIndex].agentIndex = agentIndices[agentIndex];
    }

    double objectOnlyScoreTerms() const
    {
        return wallDistScore * SynthScore::wallDistScale() +
            wallRotationScore * SynthScore::wallRotationScale() +
            //collisionBBoxScore * SynthScore::collisionScale() +
            collisionScore * SynthScore::collisionScale() +
            overhangScore * SynthScore::overhangScale() +
            scanPlacementScore * SynthScore::scanObjectScale();
    }

    float rotation;

    //ScanLocalizedObject supportingScanObject;
    double scanPlacementScore;

    double acceptable;

    double overhang;
    double overhangScore;

    double leftRightScore;

    double wallDist;
    double wallDistScore;

    double wallRotation;
    double wallRotationScore;

    double collisionScore;
    double collisionBBoxScore;

    vector<ObjectScoreAgentEntry> agents;
};

inline bool operator < (const ObjectScoreEntry &a, const ObjectScoreEntry &b)
{
    return a.rotation < b.rotation;
}

struct PlacementLocation
{
    float closestRotation(float rotation)
    {
        if (rotations.size() == 0)
            return 360.0f;
        return ml::util::minValue(rotations, [&](const ObjectScoreEntry &e)
        {
            return ml::math::min(fabs(rotation - e.rotation + 0.0f),
                fabs(rotation - e.rotation + 360.0f),
                fabs(rotation - e.rotation - 360.0f));
        });
    }

    ml::TriMeshSampler<float>::Sample sample;
    vector<ObjectScoreEntry> rotations;
};

struct CandidateObject
{
    CandidateObject()
    {
        isAgentSupport = false;
    }

    DisembodiedObject object;

    //
    // The active object may be bound to multiple agents
    //
    vector<UINT> agents;

    bool isAgentSupport;

    UINT parentObjectIndex;

    //
    // locations in the target scan where this model is likely to be found
    //
    const ScanLocatorResults *scanLocations;
    
    //
    // The locations being considered as possible placement sites
    //
    vector<PlacementLocation> locations;
};
