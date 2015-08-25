
#include "libsynth.h"

void CategoryDatabase::makeHaoInstances(ModelDatabase &models, const string &categoryName)
{
    Category &c = categories[categoryName];

    static const float tableHeight = 0.675f;
    static const float chairHeight = 0.9f;

    float objectHeight = 1.0f;

    if (categoryName == "Table")
        objectHeight = tableHeight;
    if (categoryName == "Chair")
        objectHeight = chairHeight * ml::util::randomUniform(0.8f, 1.2f);

    for (auto &mPair : models.models)
    {
        Model &m = mPair.second;
        
        if (ml::util::startsWith(mPair.first, "hao." + categoryName + "."))
        {
            DisembodiedObject instance;

            instance.agentFace = ml::vec2f(0.0f, 1.0f);
            instance.contactType = ContactType::ContactUpright;
            instance.coordianteFrameRotation = 0.0f;
            instance.distanceToWall = 0.0f;
            instance.model = &m;
            instance.modelDown = -vec3f::eZ;
            instance.modelId = m.id;
            instance.modelNormal = vec3f::eZ;
            instance.scale = objectHeight / m.bbox.getExtentZ();

            c.instances.push_back(instance);
        }
    }

    UINT maxCategoryInstanceCount = synthParams().maxCategoryInstanceCount;

    if (categoryName == "Table")
        maxCategoryInstanceCount = 200;

    if (c.instances.size() >= maxCategoryInstanceCount)
    {
        std::random_shuffle(c.instances.begin(), c.instances.end());
        c.instances.resize(maxCategoryInstanceCount);
    }
}

void CategoryDatabase::loadFromScenes(const SceneDatabase &scenes)
{
    cout << "Creating categories from scenes..." << endl;

    for (const auto &p : scenes.scenes)
    {
        for (const auto &o : p.second.objects)
        {
            const string category = o.object.model->categoryName;
            if (categories.count(category) == 0)
            {
                categories[category].name = category;
            }

            Category &c = categories[category];
            c.instances.push_back(o.object);
            c.parentCategoryFrequencies[o.parentCategoryName()]++;

            c.contactTypeFrequencies[o.object.contactType]++;

            //
            // this assumes that each category has only a single contact type
            //
            c.contactType = o.object.contactType;

            c.averageBBoxDiagonalLength += o.worldBBox.getDiagonalLength();
        }
    }

    for (auto &category : categories)
        category.second.averageBBoxDiagonalLength /= (double)category.second.instances.size();

    cout << categories.size() << " categories created" << endl;
}

void CategoryDatabase::rescaleFromModelScaleFile(const string &filename)
{
    cout << "rescaling models from " << filename << endl;
    auto lines = ml::util::getFileLines(filename);

    map<string, float> modelScaleById;
    for (UINT lineIndex = 1; lineIndex < lines.size(); lineIndex++)
    {
        //wss.383955142f43ca0b4063d41fae33f144, 0.026278285548332416, 1, sceneScales, , diagonal
        auto parts = ml::util::split(lines[lineIndex], ',');
        if (parts.size() >= 2)
        {
            modelScaleById[parts[0]] = ml::convert::toFloat(parts[1]);
        }
    }

    for (auto &c : categories)
    {
        for (auto &i : c.second.instances)
        {
            if (modelScaleById.count(i.modelId) > 0)
            {
                i.scale = modelScaleById[i.modelId];
            }
            else if (!ml::util::startsWith(i.modelId, "wss.room"))
            {
                cout << "model id not found: " << i.modelId << endl;
            }
        }
    }
}

void CategoryDatabase::output(const string &baseDir) const
{
    ml::util::makeDirectory(baseDir + "categories/");
    for (const auto &p : categories)
    {
        p.second.output(baseDir + "categories/" + p.first + ".html");
    }
}

void CategoryDatabase::loadCategoryPropertyList(const string &filename, const std::function<bool&(Category &c)> &prop)
{
    for (const string &s : ml::util::getFileLines(filename, 2))
    {
        if (categories.count(s) == 0)
            cout << "category not found: " << s << endl;
        else
            prop(categories[s]) = true;
    }
}
