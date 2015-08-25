
class Vizzer : public ml::ApplicationCallback
{
public:
    void registerEventHandlers(ml::ApplicationData& app);
    void init(ml::ApplicationData& app);
    void render(ml::ApplicationData& app);
    void keyDown(ml::ApplicationData& app, UINT key);
    void keyPressed(ml::ApplicationData& app, UINT key);
    void mouseDown(ml::ApplicationData& app, ml::MouseButtonType button);
    void mouseMove(ml::ApplicationData& app);
    void mouseWheel(ml::ApplicationData& app, int wheelDelta);
    void resize(ml::ApplicationData& app);

private:
    void loadScene(ml::ApplicationData& app, const string &sceneName);
    void batchPBRTRendering(ml::ApplicationData& app);
    void savePBRTCamera(const string& filename);

    ml::D3D11Font m_font;
    ml::FrameTimer m_timer;
    
    AppState state;
};
