
//
// SynthesizerAgent is extra per-agent information beyond what is contained in the corresponding Agent object in the scene
//
struct SynthesizerAgent
{
    SynthesizerAgent()
    {
        supportObjectPlaced = false;
    }

    Agent *agent;

    bool supportObjectPlaced;
    DisembodiedObject supportObject;

    vector<DisembodiedObject> objectsToPlace;
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
        collisionScore = -1.0f;
        collisionBBoxScore = -1.0f;
        wallDistScore = -1.0f;
        wallRotationScore = -1.0f;
        agents.resize(agentIndices.size());
        for (UINT agentIndex = 0; agentIndex < agentIndices.size(); agentIndex++)
            agents[agentIndex].agentIndex = agentIndices[agentIndex];
    }

    double objectOnlyScoreTerms() const
    {
        return wallDistScore * SynthScore::wallDistScale() +
               wallRotationScore * SynthScore::wallRotationScale() +
               (collisionScore + collisionBBoxScore) * SynthScore::collisionScale();
    }

    float rotation;

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
            return ml::math::min(fabs(rotation - e.rotation + 0.0f  ),
                                 fabs(rotation - e.rotation + 360.0f),
                                 fabs(rotation - e.rotation - 360.0f));
        });
    }

    ml::TriMeshSampler<float>::Sample sample;
    vector<ObjectScoreEntry> rotations;
};

struct ActiveObject
{
    DisembodiedObject object;

    //
    // The active object may be bound to multiple agents
    //
    vector<UINT> agents;

    //
    // If multiple agents have this object bound, it must be the support for all such agents
    //
    bool isAgentSupport;

    UINT parentObjectIndex;

    //
    // the ActiveObject with the highest score is the object that will next be inserted into the scene
    //
    double score;

    //
    // the index into the objectsToPlace vector for this object. Only relevant in selectNextObject().
    // there is an entry here for each agent.
    //
    vector<UINT> objectsToPlaceIndex;
};

namespace ml
{
    class D3D11GraphicsDevice;
}
class AssetManager;

struct Synthesizer
{
    void start(const Database &_database, ml::D3D11GraphicsDevice &graphics, AssetManager &assets, const string &architectureModelId, float architectureScale, const Scan *targetScan);

    void outputState(const string &filename) const;

    //
    // advances one step in the synthesis process. A stepping approach is used so it is easier to visualize
    // the intermediate results.
    //
    void step();

    void placeAgents();
    void selectAgentObjects();
    void selectNextObject();
    void evaluatePlacementA();
    void evaluatePlacementB();
    void placeObject();

    void makeLocations();

    double collisionBBoxScore(const DisembodiedObject &object, const mat4f &modelToWorld) const;
    double collisionScore(const DisembodiedObject &object, const mat4f &modelToWorld) const;

    mat4f makeModelToWorld(const DisembodiedObject &object, const PlacementLocation &location, float rotation) const;

    void chooseLocation(int &bestLocationIndex, int &bestRotationIndex) const;

    void makeInteractionMapRays(const DisembodiedObject &object, const mat4f &modelToWorld, const Agent &agent, const string &interactionMapName, InteractionRays &result) const;

    void updateScores();
    double scoreArrangement() const;
    double scoreArrangement(const ObjectScoreEntry &activeScore, const string &modelId, const mat4f &modelToWorld) const;
    void makeObjectScoreEntry(const DisembodiedObject &object, const mat4f &modelToWorld, const vec3f &contactPos, ObjectScoreEntry &result);

    Scene scene;

    const Database *database;
    ml::D3D11GraphicsDevice *graphics;
    AssetManager *assets;

    const Scan *targetScan;

    //
    // stage indicates the next action that will be taken when step() is called
    //
    SynthesizerStage stage;

    vector<SynthesizerAgent> agents;

    //
    // When an object is being placed, it is removed from the Agent's objectsToPlace array and loaded
    // as the activeObject
    //
    ActiveObject active;
    
    //
    // The locations being considered as possible placement sites
    //
    vector<PlacementLocation> locations;

    vector<ObjectScoreEntry> scores;

    bool done;
};
