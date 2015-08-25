
struct ResultsMode : public VizzerModeInterface
{
    void init(ml::ApplicationData& _app, AppState &_state);
    void newScene(const string &filename);
    void render();
    void renderPBRT(const string& path);
    void keyDown(UINT key);
    void mouseDown(ml::MouseButtonType button);
    
    void sortAllResults();
    void sortResultsDirectory(const string &sourceDir, const string &targetDir, const function<double(const SceneScorecard&)> &valueFunction);

    void renderResultsFolder(const string &dir) const;
    void renderSceneD3D(const string &sceneFilename, const string &outputFilename) const;

    string helpText() const;
    vector< pair<string, RGBColor> > statusText() const;

    ml::ApplicationData *app;
    AppState *state;

    int selectedObjectIndex;

    Scene resultScene;

    vector<int> objectState;

    vector< ml::TriMeshAcceleratorBruteForcef > objectAccelerators;
};
