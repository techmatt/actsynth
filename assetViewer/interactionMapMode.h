
struct InteractionMapMode : public VizzerModeInterface
{
    void init(ml::ApplicationData& _app, AppState &_state);
    void loadModel(const string &modelId);

    void render();
    void keyDown(UINT key);
    void keyPressed(UINT key);
    void mouseDown(ml::MouseButtonType button);
    void mouseWheel(int wheelDelta);
    void mouseMove();
    void sampleCylinderPoints();

    void recategorizeModel(const string &newName) const;

    string helpText() const;
    vector< pair<string, RGBColor> > statusText() const;

    ml::ApplicationData *app;
    AppState *state;

    ml::Cameraf camera;

    vector<string> models;
    int modelIndex;

    vector<string> interactionTypes;
    int interactionIndex;

    const Model *model;
    mat4f modelToWorld;

    int selectedCentroidIndex;

    ml::TriMeshAcceleratorBruteForcef accelerator;

    bool dirty;
};
