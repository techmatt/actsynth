
struct SceneDatabase
{
	// Load scenes from WebSceneStudio json format file
    void load(const string &scenesCSVFile, const string &sceneListFile, const string &synthesisDataDirectory, int maxScenes = -1);
    void updateModelReferences(ModelDatabase &models);
    void loadObjectData();
    void loadAgentAnnotations(const string &path);

    const Scene& getScene(const string &sceneName) const
    {
        if (scenes.count(sceneName) == 0)
        {
            cout << "scene not found: " << sceneName << endl;
            return scenes.begin()->second;
        }
        return scenes.find(sceneName)->second;
    }

    Scene& getScene(const string &sceneName)
    {
        if (scenes.count(sceneName) == 0)
        {
            cout << "scene not found: " << sceneName << endl;
            return scenes.begin()->second;
        }
        return scenes.find(sceneName)->second;
    }

    void output(const string &baseDir) const;
    vector<const Agent*> agents() const;
    vector<const Agent*> agents(const string &activityFilter) const;

    map<string, Scene> scenes;
};
