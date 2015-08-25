
#include "libsynth.h"

const DisembodiedObject& Category::findFirstInstance(const string &modelId) const
{
    for (const DisembodiedObject &o : instances)
    {
        if (o.modelId == modelId)
            return o;
    }
    cout << "model not found: " << modelId << endl;
    return instances[0];
}

float firstScale = 0.0f;
const DisembodiedObject& Category::sampleInstance() const
{
    while (true)
    {
        const auto &result = ml::util::randomElement(instances);
        if (name == "Table" && result.modelId != "wss.eed2fa156d662c79b26e384cea2f274e")
            continue;

        if (name == "Table" && result.modelId == "wss.eed2fa156d662c79b26e384cea2f274e" && firstScale == 0.0f)
        {
            firstScale = 1.0f;
            DisembodiedObject *x = (DisembodiedObject *)&result;
            x->scale *= 1.1f;
        }

        if (!result.model->blacklisted)
            return result;
    }
}

vector<const DisembodiedObject*> Category::sampleInstances(UINT instanceCount) const
{
    const UINT sampleMaxAttempts = 100;

    vector<const DisembodiedObject*> result;
    set<string> modelIDs;
    for (UINT sampleIndex = 0; sampleIndex < sampleMaxAttempts; sampleIndex++)
    {
        const DisembodiedObject &sample = sampleInstance();
        if (modelIDs.count(sample.model->id) == 0)
        {
            result.push_back(&sample);
            if (result.size() == instanceCount)
                return result;
            modelIDs.insert(sample.model->id);
        }
    }
    return result;
}

void Category::output(const string &filename) const
{
    HTMLExporter exporter(filename, "Category: " + name);

    exporter.appendHeader1("parent category frequencies");
    for (const auto &p : parentCategoryFrequencies)
        exporter.appendBoldText(p.first + " - ", to_string(p.second));

    exporter.appendHeader1("contact type frequencies: " + contactTypeToString(contactType));
    for (const auto &p : contactTypeFrequencies)
        exporter.appendBoldText(contactTypeToString(p.first) + " - ", to_string(p.second));

    int oIndex = 0;
    while (true)
    {
        ml::Grid2<string> grid(1, 10);

        for (int i = 0; i < 10; i++)
        if (oIndex < instances.size())
            grid(0, i) = instances[oIndex++].model->imageHTML();
        
        exporter.appendTable("", grid);

        if (oIndex == instances.size())
            break;

        //TODO: export X-Y-Z size of object instances
    }

    exporter.appendHeader1("wall distance entries");
    vector<float> wallDistances;
    for (const auto &p : instances)
        wallDistances.push_back(p.distanceToWall);

    std::sort(wallDistances.begin(), wallDistances.end());
    for (float dist : wallDistances)
        exporter.appendText(to_string(dist));
    //ml::convert::toString(wallDistances);
}
