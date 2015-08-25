
struct SceneDatabase;
struct ModelDatabase;

struct CategoryDatabase
{
    void loadFromScenes(const SceneDatabase &scenes);

    void makeHaoInstances(ModelDatabase &models, const string &categoryName);

    void output(const string &baseDir) const;

    void rescaleFromModelScaleFile(const string &filename);

    void loadCategoryPropertyList(const string &filename, const std::function<bool&(Category &c)> &prop);

    const Category& getCategory(const string &category) const
    {
        if (categories.count(category) == 0)
        {
            cout << "!!! category not found: " << category << endl;
            return categories.find("Chair")->second;
        }
        return categories.find(category)->second;
    }
    map<string, Category> categories;
};
