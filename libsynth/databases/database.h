

struct Database
{
    void loadAll();
    void output(const string &baseDir) const;

    //
    // databases that are loaded from disk
    //
    ModelDatabase models;
    SceneDatabase scenes;
    ScanDatabase scans;
    InteractionMapDatabase interactionMaps;

    //
    // databases that are computed from the other databases
    //
    CategoryDatabase categories;
    ActivityDatabase activities;
};
