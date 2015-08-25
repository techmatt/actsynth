
enum VizzerField
{
    VizzerFieldCombinedScore,
    VizzerFieldWallDistance,
    VizzerFieldCollision,
    VizzerFieldCollisionBBoxOnly,
    VizzerFieldScanSupport,
    VizzerFieldDistributionBase,
    VizzerFieldVisibilityGaze,
    VizzerFieldVisibilityFintertip,
    VizzerFieldVisibilityBackSupport,
    VizzerFieldCount
};

inline string fieldName(VizzerField field)
{
    switch (field)
    {
    case VizzerFieldCombinedScore: return "combined score";
    case VizzerFieldWallDistance: return "wall distance";
    case VizzerFieldCollision: return "collision";
    case VizzerFieldCollisionBBoxOnly: return "collision bbox";
    case VizzerFieldScanSupport: return "scan support";
    case VizzerFieldDistributionBase: return "base distribution";
    case VizzerFieldVisibilityGaze: return "gaze visibility";
    case VizzerFieldVisibilityFintertip: return "fingertips visibility";
    case VizzerFieldVisibilityBackSupport: return "back visibility";
    default: return "unknown field";
    }
}

inline VizzerField fieldFromName(const string &name)
{
    if (name == "WallDistance") return VizzerFieldWallDistance;
    if (name == "DistributionBase") return VizzerFieldDistributionBase;
    if (name == "VisibilityGaze") return VizzerFieldVisibilityGaze;
    cout << "unrecognized field: " << name << endl;
    return VizzerFieldWallDistance;
}

struct FieldMode : public VizzerModeInterface
{
    void init(ml::ApplicationData& _app, AppState &_state);
    void render();
    void renderCandidateObject();
    void keyDown(UINT key);
    void mouseDown(ml::MouseButtonType button);

    void updateColumns();

    void updateColumnVisualization();
    void updateRayVisualization();
    //void updateScanObjectVisualization();

    void viewMax(int candidateObjectFilter);

    string helpText() const;
    vector< pair<string, RGBColor> > statusText() const;

    bool candidateValid() const;

    ml::ApplicationData *app;
    AppState *state;

    ml::D3D11TriMesh rayVisualization;
    ml::D3D11TriMesh columnVisualization;

    ColumnRepresentation columns;

    int selectedCandidateIndex;
    int selectedLocationIndex;
    int selectedRotationIndex;
    int selectedAgentIndex;

    VizzerField field;

    bool rayVisualizationDirty;

    ml::ColorGradient gradient;
};
