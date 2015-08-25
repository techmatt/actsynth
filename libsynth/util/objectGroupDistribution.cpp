
#include "libsynth.h"

ObjectGroupDistributionIndependent::ObjectGroupDistributionIndependent(const vector< vector<DisembodiedObject> > &samples, const CategoryDatabase &categories)
{
    _samples = samples;
    _categories = &categories;

    for (const auto &s : samples)
    {
        for (const DisembodiedObject &e : s)
        {
            if (_categoryFrequencies.count(e.model->categoryName) == 0)
                _categoryFrequencies[e.model->categoryName] = vector<int>();
        }
    }


    for (const auto &s : samples)
    {
        map<string, int> categoryCounts;
        for (const DisembodiedObject &e : s)
        {
            string category = e.model->categoryName;
            if (categoryCounts.count(category) == 0)
                categoryCounts[category] = 0;
            categoryCounts[category]++;
        }
        for (auto &c : _categoryFrequencies)
        {
            int count = 0;
            if (categoryCounts.count(c.first) != 0)
                count = categoryCounts[c.first];
            c.second.push_back(count);
        }
    }

    for (auto &c : _categoryFrequencies)
    {
        _categoryList.insert(c.first);
        std::sort(c.second.begin(), c.second.end());
    }
}

double ObjectGroupDistributionIndependent::probabilityCategoryCountAtLeast(const string &categoryName, int categoryCount) const
{
    if (_categoryFrequencies.count(categoryName) == 0)
        return 0.0;

    const vector<int> &counts = _categoryFrequencies.find(categoryName)->second;
    double positiveCount = 0;
    double count = 0;
    for (int index = 0; index < counts.size(); index++)
    {
        count++;
        if (counts[index] >= categoryCount)
            positiveCount++;
    }
    return positiveCount / count;
}

vector<DisembodiedObject> ObjectGroupDistributionIndependent::sample() const
{
    vector<DisembodiedObject> result;

    for (auto &c : _categoryFrequencies)
    {
        int count = ml::util::randomElement(c.second);
        for (int objectIndex = 0; objectIndex < count; objectIndex++)
        {
            const Category &category = _categories->getCategory(c.first);
            const DisembodiedObject &instance = ml::util::randomElement(category.instances);

            result.push_back(instance);
        }
    }

    return result;
}

ObjectGroupDistributionMemorizer::ObjectGroupDistributionMemorizer(const vector< vector<DisembodiedObject> > &samples)
{
    _samples = samples;
    for (const auto &sample : samples)
    {
        for (const DisembodiedObject &object : sample)
            _categoryList.insert(object.model->categoryName);
    }
}

double ObjectGroupDistributionMemorizer::probabilityCategoryCountAtLeast(const string &categoryName, int categoryCount) const
{
    int positiveCount = 0;
    for (const vector<DisembodiedObject> &objects : _samples)
    {
        int count = 0;
        for (const DisembodiedObject &object : objects)
        {
            if (object.model->categoryName == categoryName)
                count++;
        }
        if (count >= categoryCount)
            positiveCount++;
    }
    return double(positiveCount) / double(_samples.size());
}

void ObjectGroupDistributionMemorizer::output(HTMLExporter &e, const string &name) const
{
    DisembodiedObject::output(e, name + " samples", _samples);
}

void ObjectGroupDistributionIndependent::output(HTMLExporter &e, const string &name) const
{
    e.appendHeader1(name);

    for (auto &p : _categoryFrequencies)
    {
        e.file << "<p><b>" << p.first << ": </b>";
        for (int f : p.second)
            e.file << f << ",";
        e.file << endl;
    }

    e.appendLineBreak();

    DisembodiedObject::output(e, name + " samples", _samples);
}