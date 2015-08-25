
enum VizzerMode
{
    VizzerModeAgentPlacement,
    VizzerModeAgentOwnership,
    VizzerModeInteractionMap,
    VizzerModeScore,
    VizzerModeScan,
    VizzerModeResults,
    VizzerModeHeatmap,
    VizzerModeCount
};

inline string modeName(VizzerMode mode)
{
    switch (mode)
    {
    case VizzerModeAgentPlacement: return "agent placement";
    case VizzerModeAgentOwnership: return "agent ownership";
    case VizzerModeInteractionMap: return "interaction map";
    case VizzerModeScore: return "score";
    case VizzerModeScan: return "scan";
    case VizzerModeResults: return "results";
    case VizzerModeHeatmap: return "heatmap";
    default: return "unknown mode";
    }
}

inline VizzerMode modeFromName(const string &name)
{
    if (name == "AgentPlacement") return VizzerModeAgentPlacement;
    if (name == "AgentOwnership") return VizzerModeAgentOwnership;
    if (name == "InteractionMap") return VizzerModeInteractionMap;
    if (name == "Score") return VizzerModeScore;
    if (name == "Scan") return VizzerModeScan;
    if (name == "Results") return VizzerModeResults;
    if (name == "Heatmap") return VizzerModeHeatmap;
    cout << "unrecognized mode : " << name << endl;
    return VizzerModeAgentPlacement;
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
    }

    void updateModeInterface()
    {
        modeInterface = NULL;
        if (mode == VizzerModeAgentPlacement) modeInterface = &agentPlacementMode;
        if (mode == VizzerModeAgentOwnership) modeInterface = &agentPlacementMode;
        if (mode == VizzerModeInteractionMap) modeInterface = &interactionMapMode;
        if (mode == VizzerModeScore) modeInterface = &scoreMode;
        if (mode == VizzerModeScan) modeInterface = &scanMode;
        if (mode == VizzerModeResults) modeInterface = &resultsMode;
        ui.sendMessage("helpText " + ml::util::replace(modeInterface->helpText(), ' ', '@'));
    }

    ml::UIConnection ui;
    EventMap eventMap;

    AssetRenderer renderer;
    AssetManager assets;
    Database database;

    bool showBBoxes;

    ml::Camera<float> camera;

    string curScene;

    vector<ml::D3D11TriMesh> objectBBoxes;

    VizzerMode mode;
    AgentMode agentPlacementMode;
    InteractionMapMode interactionMapMode;
    ScoreMode scoreMode;
    ScanMode scanMode;
    ResultsMode resultsMode;

    VizzerModeInterface *modeInterface;
};
