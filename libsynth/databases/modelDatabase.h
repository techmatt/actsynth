
//
// ModelDatabase only stores models for the perspective of synthesis, which requires only vertex positions (although TriMesh
// unfortunately cannot be configured to only store vertices, talk to Matthias about this.)
//
// Texture information etc. is only needed by Direct3D, which is intended to only live in an asset maanger in the synthesis project.
//
struct ModelDatabase
{
    static void convertToSimpleTsv(const string &fileIn, const string &fileOut);
    void loadFull(const string &modelsTsvFile);
    void loadSimple(const string &modelsTsvFile);
    void overrideCategories(const string &modelsTsvFile);
    void loadBlacklist(const string &blacklistFile);

    void loadHaoCategory(const string &categoryName);
    void computeHaoBBoxes(const string &categoryName);

    void saveReducedModelSet(const string &modelsTsvFile, const SceneDatabase &scenes) const;
    void output(const string &baseDir) const;
    const Model& getModel(const string &modelId) const;

    //
    // erases all model data.
    //
    void clearAllModels();

    //
    // all possible models are given an entry at load time, however they will not have their meshes populated until
    // Model::loadMesh is called. The meshes may be unloaded at any time.
    //
    map<string, Model> models;
};
