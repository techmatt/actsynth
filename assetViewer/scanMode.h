
struct ScanMode : public VizzerModeInterface
{
    void init(ml::ApplicationData& _app, AppState &_state);
    
    void saveScan();
    void newScan();
    
    void render();
    void keyPressed(UINT key);
    void keyDown(UINT key);
    void mouseDown(ml::MouseButtonType button);

    void renderSceneD3D(const string &outputFilename) const;

    string helpText() const;
    vector< pair<string, RGBColor> > statusText() const;

    ml::ApplicationData *app;
    AppState *state;

    vector<string> activityList;
    map<string, vec3f> activityColors;

    int selectedAgentIndex;
    int scanIndex;

    ml::D3D11TriMesh scanMesh;

    vector< ml::TriMeshAcceleratorBruteForcef > objectAccelerators;

    bool dirty;
    bool renderAgents;
};
