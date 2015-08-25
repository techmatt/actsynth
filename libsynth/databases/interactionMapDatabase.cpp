
#include "libsynth.h"

void InteractionMapDatabase::load(ModelDatabase &models, const string &path)
{
    ml::Directory dir(path);
    for (const string &filename : dir.files())
    {
        if (ml::util::endsWith(filename, ".arv"))
        {
            const string modelId = ml::util::remove(filename, ".arv");

            if (models.models.count(modelId) == 0)
            {
                cout << "Interaction map model not found: " << modelId << endl;
            }
            else
            {
                InteractionMap &entry = maps[modelId];
                entry.load(dir.path() + filename);
            }
        }
    }
}

vector<string> InteractionMapDatabase::allInteractionMapTypes() const
{
    //std::set<string> data;
    //for (const auto &p : maps)
    //    for (const auto &i : p.second.entries)
    //        data.insert(i.first);

    //return vector<string>(data.begin(), data.end());

    vector<string> result;
    result.push_back("gaze");
    result.push_back("feet");
    result.push_back("fingertip");
    result.push_back("backSupport");
    result.push_back("hips");
    return result;
}

bool InteractionMapDatabase::hasInteractionMapEntry(const string &modelID, const string &interactionMapType) const
{
    if (maps.count(modelID) == 0)
        return false;
    return (maps.find(modelID)->second.entries.count(interactionMapType) != 0);
}

const InteractionMapEntry& InteractionMapDatabase::getInteractionMapEntry(const string &modelID, const string &interactionMapType) const
{
    return maps.find(modelID)->second.entries.find(interactionMapType)->second;
}

InteractionMapEntry& InteractionMapDatabase::createInteractionMapEntry(const string &modelID, const string &interactionMapType)
{
    auto &result = maps[modelID].entries[interactionMapType];
    result.type = interactionMapType;
    return result;
}

void InteractionMapDatabase::output(const ModelDatabase &models, const string &baseDir) const
{
    std::ofstream file(baseDir + "interactionMaps.csv");
    
    auto types = allInteractionMapTypes();
    
    file << "id,category";
    for (const string &s : types)
        file << "," << s;
    file << endl;

    for (const auto &p : maps)
    {
        const Model &model = models.getModel(p.first);
        file << model.id << "," << model.categoryName;

        for (const string &s : types)
            file << "," << p.second.entries.count(s);

        file << endl;
    }
}
