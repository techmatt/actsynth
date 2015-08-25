
struct ScanSegmentation
{
    vector<MeshSegment> segments;
};

struct Scan
{
    void load(const string &meshFilename, const string &alignmentFilename, const string &agentFilename);

    //void loadAlignment(const string &filename);
    void saveAlignment(const string &filename) const;

    void loadAgentAnnotations(const string &filename);
    void saveAgentAnnotations(const string &filename) const;

    string name;
    string pathName;

#ifdef _DEBUG
    static const UINT segmentationCount = 1;
#else
    static const UINT segmentationCount = 3;
#endif
    ScanSegmentation segmentations[segmentationCount];

    const ScanSegmentation& getSegmentationAtResolution(float maxDimension) const;

    TriMeshf baseMesh;
    TriMeshf transformedMesh;

    //
    // agent annotations come either from scenegrok or manual annotations
    //
    vector<Agent> agents;

    //
    // alignment from local mesh coordinates to scene
    //
    vec3f translation;
    float rotation;
    mat4f transform;

    ml::BinaryGrid3 voxelization;
    ml::DistanceField3f distanceField;
    mat4f scanToVoxel;
};
