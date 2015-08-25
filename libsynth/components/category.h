
struct Category
{
    Category()
    {
        isHandOrientation = false;
        isObjectSupport = false;
        isAgentSupport = false;
        isShared = false;
        isTopSupport = false;
        isBBoxCollision = false;
        isLeftRight = false;
        isNoSimplify = false;
        isDiversity = false;
        isNonEssential = false;
        averageBBoxDiagonalLength = 0.0f;
    }

    void output(const string &filename) const;
    const DisembodiedObject& sampleInstance() const;
    vector<const DisembodiedObject*> sampleInstances(UINT instanceCount) const;
    const DisembodiedObject& findFirstInstance(const string &modelId) const;

    string name;
    vector<DisembodiedObject> instances;
    
    map<string, int> parentCategoryFrequencies;
    
    map<ContactType, int> contactTypeFrequencies;
    ContactType contactType;

    bool isHandOrientation;
    bool isObjectSupport;
    bool isAgentSupport;
    bool isShared;
    bool isTopSupport;
    bool isBBoxCollision;
    bool isLeftRight;
    bool isNoSimplify;
    bool isDiversity;
    bool isNonEssential;

    double averageBBoxDiagonalLength;
};
