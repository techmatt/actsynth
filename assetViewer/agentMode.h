
struct AgentMode : public VizzerModeInterface
{
    void init(ml::ApplicationData& _app, AppState &_state);
    void closeScene();
    void newScene();
    void render();
    void renderPBRT(const string &path);
    void keyDown(UINT key);
    void mouseDown(ml::MouseButtonType button);

    void assignProximityOwnership();

    string helpText() const;
    vector< pair<string, RGBColor> > statusText() const;

    ml::ApplicationData *app;
    AppState *state;

    vector<string> activityList;
    map<string, vec3f> activityColors;

    vector<vec3f> agentColors;
    vector<vec3f> agentColorList;

    int selectedAgentIndex;

    vector< ml::TriMeshAcceleratorBruteForcef > objectAccelerators;

    bool dirty;
};
