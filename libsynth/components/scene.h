
struct Database;

struct SceneScorecard
{
    SceneScorecard()
    {
        memset(this, 0, sizeof(SceneScorecard));
    }
    double scanGeometry;
    double objectCount;
    double worstGaze;
    double worstOffsetScore;
    double averageOffsetScore;
    /*double placeholder0;
    double placeholder1;
    double placeholder2;
    double placeholder3;
    double placeholder4;*/
};

struct Scene
{
    void output(const string &filename) const;

    void save(const string &filename) const;
    void load(const string &filename, const Database &database);

    void loadAgentAnnotations(const string &filename);
    void saveAgentAnnotations(const string &filename) const;

    void updateAgentFaceDirections();

    void computeWallSegments();

    vector<UINT> Scene::agentsFromObjectIndex(UINT objectIndex) const;

	float distanceToWall(const OBBf &bbox) const;

    vector<UINT> objectsWithCategory(const string &categoryName, int agentBinding) const;

    string id;
    string sourceFilename;

    // the first ObjectInstance is always the base architecture
    vector<ObjectInstance> objects;

    //
    // Agents includes activityClusters, which are denoted by the id.
    // Example: diningTableCluster is the ActivityCluster composed of diningTable activities.
    // each Agent has a set of ModelInstances bound to it.
    //
    vector<Agent> agents;

    ml::bbox3f bbox;

    SceneScorecard scorecard;

    vector<ml::LineSegment2f> wallSegments;
};
