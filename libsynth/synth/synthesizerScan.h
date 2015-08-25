
namespace ml
{
    class D3D11GraphicsDevice;
}
class AssetManager;

struct Synthesizer
{
    void start(const Database &_database, ml::D3D11GraphicsDevice &graphics, AssetManager &assets, const SceneTemplate &_sceneTemplate, const string &architectureModelId, float architectureScale);

    void outputState(const string &filename) const;

    //
    // advances one step in the synthesis process. A stepping approach is used so it is easier to visualize
    // the intermediate results.
    //
    void step();
    void synthesize();

    void placeAgents();
    void addAgentActivityBlockers();
    void sampleAgentObjects();
    void chooseCandidateObjects();
    void evaluatePlacementA();
    void evaluatePlacementB();
    void placeBestObject();

    void makeLocations();
    void makeLocations(CandidateObject &candidate);

    double collisionBBoxScore(const DisembodiedObject &object, const mat4f &modelToWorld) const;
    double collisionScore(const DisembodiedObject &object, const mat4f &modelToWorld) const;

    mat4f makeModelToWorld(const DisembodiedObject &object, const PlacementLocation &location, float rotation) const;

    void chooseObjectAndLocation(int candidateObjectFilter, int &candidateObjectIndex, int &bestLocationIndex, int &bestRotationIndex, bool silentMode) const;

    void makeInteractionMapRays(const DisembodiedObject &object, const mat4f &modelToWorld, const Agent &agent, const string &interactionMapName, InteractionRays &result) const;

    void updateAgentCategories();

    void updateBaseColumns();

    void updateObjectScores();
    void updateScorecard();

    double scoreArrangement() const;
    double scoreArrangement(const ObjectScoreEntry &activeScore, const string &modelId, const mat4f &modelToWorld) const;

    void computeBaseOffset(const DisembodiedObject &object, const mat4f &modelToWorld, const vec3f &contactPos, ObjectScoreEntry &result);
    void computeLeftRight(const DisembodiedObject &object, const vec3f &contactPos, ObjectScoreEntry &result);
    void computeOverhang(const DisembodiedObject &object, const ObjectInstance &parent, const mat4f &modelToWorld, ObjectScoreEntry &result);
    void computeInteractionRays(const DisembodiedObject &object, const mat4f &modelToWorld, ObjectScoreEntry &result);
    void computeCollision(const DisembodiedObject &object, const mat4f &modelToWorld, ObjectScoreEntry &result);
    void computeScanPlacement(const DisembodiedObject &object, const mat4f &modelToWorld, ObjectScoreEntry &result);
    void computeAllTerms(const DisembodiedObject &object, const ObjectInstance &parent, const mat4f &modelToWorld, const vec3f &contactPos, ObjectScoreEntry &result);

    Scene scene;

    const Database *database;
    ml::D3D11GraphicsDevice *graphics;
    AssetManager *assets;

    const SceneTemplate *sceneTemplate;

    //ScanLocator locator;

    //
    // stage indicates the next action that will be taken when step() is called
    //
    SynthesizerStage stage;

    vector<SynthesizerAgent> agents;

    //
    // When an object is being placed, it is removed from the Agent's objectsToPlace array and loaded
    // as the activeObject
    //
    vector<CandidateObject> candidateObjects;

    vector<ObjectScoreEntry> objectScores;

    vector<ActivityBlocker> activityBlockers;

    ColumnRepresentation baseColumns, columnsTemp;

    bool earlyTermination;
    bool verbose;
};
