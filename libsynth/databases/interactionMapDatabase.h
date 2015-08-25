
struct InteractionMapDatabase
{
    void load(ModelDatabase &models, const string &path);

    vector<string> allInteractionMapTypes() const;

    void output(const ModelDatabase &models, const string &baseDir) const;

    bool hasInteractionMapEntry(const string &modelID, const string &interactionMapType) const;
    const InteractionMapEntry& getInteractionMapEntry(const string &modelID, const string &interactionMapType) const;
    InteractionMapEntry& createInteractionMapEntry(const string &modelID, const string &interactionMapType);

    //
    // interaction maps are indexed by model ID
    //
    map<string, InteractionMap> maps;
};
