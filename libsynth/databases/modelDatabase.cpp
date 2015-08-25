
#include "libsynth.h"

#include <mLibBoost.h>

const Model& ModelDatabase::getModel(const string &modelId) const
{
    if (models.count(modelId) > 0)
        return models.find(modelId)->second;
    
    cout << "Model not found: " << modelId << endl;
    return models.find("not-found")->second;
}

void ModelDatabase::saveReducedModelSet(const string &modelsTsvFile, const SceneDatabase &scenes) const
{
    //using namespace std;
    //set<string> models;
    //for (const auto &s : scenes.scenes)
    //    for(const auto &m : s.second.
}

void ModelDatabase::convertToSimpleTsv(const string &fileInPath, const string &fileOutPath)
{
    cout << "Converting model list at " << fileInPath << endl;
    auto lines = ml::util::getFileLines(util::copyToLocalCache(fileInPath), 3);

    std::ofstream fileOut(fileOutPath);
    for (const string &line : lines)
    {
        auto parts = ml::util::split(ml::util::remove(line, "\""), '\t', true);

        if (parts[0] == "fullId" || ml::util::startsWith(parts[0], "archive3d"))
            continue;

        string category = "uncategorized";

        auto categoryWords = ml::util::filter(ml::util::split(parts[3], ','), [](const string &s) { return s.size() > 1 && s[0] != '_'; });

        if (categoryWords.size() > 0)
            category = categoryWords[0];

        fileOut << parts[0] << '\t' << category << endl;
    }
}

void ModelDatabase::loadFull(const string &modelsTsvFile)
{
    const string localFilename = util::cacheDir() + "models.tsv";
    convertToSimpleTsv(modelsTsvFile, localFilename);
    loadSimple(localFilename);
}

void ModelDatabase::loadSimple(const string &modelsTsvFile)
{
    cout << "Loading model database from " << modelsTsvFile << endl;
    auto lines = ml::util::getFileLines(util::copyToLocalCache(modelsTsvFile), 3);

    for (const string &line : lines)
    {
        auto parts = ml::util::split(line, '\t');

        Model &m = models[parts[0]];

        m.id = parts[0];

        if (parts.size() >= 2 && parts[1].size() > 0)
            m.categoryName = parts[1];
    }

    models["not-found"].id = "not-found";
    models["not-found"].categoryName = "not-found";

    cout << models.size() << " models loaded" << endl;
}

void ModelDatabase::loadHaoCategory(const string &categoryName)
{
    const string haoDirectory = R"(V:\data\models\repositories\hao-shapenet\)" + categoryName;
    cout << "Loading " << categoryName << "s from " << haoDirectory << endl;
    
    for (const string &modelBaseId : ml::Directory::enumerateDirectories(haoDirectory))
    {
        const string id = "hao." + categoryName + "." + modelBaseId;

        Model &m = models[id];

        m.id = id;
        m.categoryName = categoryName;
    }

    //
    // NOTE: model bounding boxes are not initialized yet.
    //

    cout << models.size() << " models loaded" << endl;
}

void ModelDatabase::computeHaoBBoxes(const string &categoryName)
{
    cout << "Computing Hao-database bboxes..." << endl;

    map<string, bbox3f> modelIDToBBox;

    const string bboxFile = R"(V:\data\models\repositories\hao-shapenet\)" + categoryName + "_bbox.dat";

    if (ml::util::fileExists(bboxFile))
        ml::deserializeFromBoostFile(bboxFile, modelIDToBBox);

    size_t newModelCount = 0;

    for (auto &mPair : models)
    {
        if (ml::util::startsWith(mPair.first, "hao." + categoryName + "."))
        {
            if (modelIDToBBox.count(mPair.first) == 0)
            {
                newModelCount++;
                cout << "Computing model bbox for " << mPair.first << endl;

                vector<ml::TriMeshf> meshes, simplifiedMeshes;
                vector<ml::Materialf> materials;
                util::loadCachedModel(mPair.first, meshes, simplifiedMeshes, materials);

                vector<vec3f> points;
                for (const auto &mesh : meshes)
                    for (const auto &v : mesh.getVertices())
                        points.push_back(v.position);

                if (points.size() == 0)
                {
                    cout << "No points found for " << mPair.first << endl;
                }

                modelIDToBBox[mPair.first] = ml::bbox3f(points);
            }


            mPair.second.bbox = modelIDToBBox[mPair.first];
        }
    }

    if (newModelCount > 0)
    {
        cout << "New models found: " << newModelCount << endl;
        ml::serializeToBoostFile(bboxFile, modelIDToBBox);
    }
}

void ModelDatabase::overrideCategories(const string &modelsTsvFile)
{
    cout << "Overriding categories from " << modelsTsvFile << endl;
    auto lines = ml::util::getFileLines(modelsTsvFile, 3);

    for (const string &line : lines)
    {
        auto parts = ml::util::split(line, '\t');

        Model &m = models[parts[0]];

        m.id = parts[0];

        if (parts.size() >= 2 && parts[1].size() > 0)
            m.categoryName = parts[1];
    }
}

void ModelDatabase::loadBlacklist(const string &blacklistFile)
{
    cout << "Loading synthesis blacklist from " << blacklistFile << endl;
    auto lines = ml::util::getFileLines(blacklistFile, 3);

    for (const string &line : lines)
    {
        Model &m = models[line];
        m.blacklisted = true;
    }
}

void ModelDatabase::output(const string &baseDir) const
{
    std::ofstream file(baseDir + "models.csv");
    file << "id,category" << endl;
    for (const auto &p : models)
    {
        file << p.second.id << "," << p.second.categoryName << endl;
    }
}
