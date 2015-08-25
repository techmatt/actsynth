enum VizzerMode
{
    VizzerModeField,
    VizzerModeCount
};

inline string modeName(VizzerMode mode)
{
    switch (mode)
    {
    case VizzerModeField: return "view field";
    default: return "unknown mode";
    }
}

inline VizzerMode modeFromName(const string &name)
{
    if (name == "Field") return VizzerModeField;
    cout << "unrecognized mode: " << name << endl;
    return VizzerModeField;
}

struct ConstantBufferData
{
    mat4f worldViewProj;
};

struct AppState
{
    AppState()
    {
        showBBoxes = false;
        showScan = true;
        showScene = true;
        showPlanes = false;
        showHeatmap = false;
        showScanColumns = false;
        showAgents = false;
        showCollisionMesh = false;
    }

    void updateModeInterface()
    {
        modeInterface = NULL;
        if (mode == VizzerModeField) modeInterface = &fieldMode;
        ui.sendMessage("helpText " + ml::util::replace(modeInterface->helpText(), ' ', '@'));
    }

    ml::UIConnection ui;
    EventMap eventMap;

    AssetRenderer renderer;
    AssetManager assets;
    Database database;

    SceneTemplate sceneTemplate;
    Scan *targetScan;
    D3D11TriMesh targetScanMesh;
    D3D11TriMesh activityBlockersMesh;
    Synthesizer synth;

    bool showBBoxes;
    bool showScan;
    bool showScene;
    bool showPlanes;
    bool showHeatmap;
    bool showScanColumns;
    bool showAgents;
    bool showCollisionMesh;

    ml::Cameraf camera;

    VizzerMode mode;

    VizzerModeInterface *modeInterface;
    FieldMode fieldMode;
};
