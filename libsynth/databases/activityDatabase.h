
struct InteractionMapDatabase;

struct ActivityDatabase
{
    void loadFromScenes(const SceneDatabase &scenes, const CategoryDatabase &categories, const InteractionMapDatabase &interactionMaps);

    bool hasActivityCategoryDistribution(const string &activity, const string &category) const;
    const ActivityCategoryDistributions& getActivityCategoryDistribution(const string &activity, const string &category) const;

    void output(const string &baseDir) const;

    const Activity& getActivity(const string &activityName) const
    {
        auto it = activities.find(activityName);
        if (it == activities.end())
            cout << "activity not found: " << activityName << endl;
        return it->second;
    }

    map<string, Activity> activities;
    //map<string, ActivityCluster> activityClusters;

private:
    static void loadActivity(const SceneDatabase &scenes, const CategoryDatabase &categories, const InteractionMapDatabase &interactionMaps, Activity &activity);
    static ObjectGroupDistribution* makeObjectGroupDistribution(const SceneDatabase &scenes, const CategoryDatabase &categories, const string &activityName);
    static CategoryDistribution makeSupportDistribution(const SceneDatabase &scenes, const string &activityName);
    static void makeActivityCategoryDistributions(const SceneDatabase &scenes, const CategoryDatabase &categories, const InteractionMapDatabase &interactionMaps, const string &activityName, ActivityCategoryDistributions &dist);
};
