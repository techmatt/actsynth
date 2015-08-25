
struct ObjectScoreVizData
{
    ObjectScoreVizData()
    {
        mode = Normal;
    }

    enum VizMode
    {
        Normal,
        Red,
        Green,
        Hidden,
        ModeCount,
    };


    ml::D3D11TriMesh unifiedMesh;
    VizMode mode;
};

struct ScoreMode : public VizzerModeInterface
{
    void init(ml::ApplicationData& _app, AppState &_state);
    void newScene();
    void render();
    void renderPBRT(const string &path);
    void keyDown(UINT key);
    void mouseDown(ml::MouseButtonType button);
    void renderSceneD3D(const string &outputFilename) const;

    bool getFirstIntersection(const ml::Rayf &ray, ml::TriMeshRayAccelerator<float>::Intersection &intersect, UINT &objectIndex);

    string helpText() const;
    vector< pair<string, RGBColor> > statusText() const;

    ml::ApplicationData *app;
    AppState *state;

    int selectedObjectIndex;

    vector< ml::TriMeshAcceleratorBruteForcef > objectAccelerators;

    vector< ObjectScoreVizData > objectVizData;
};
