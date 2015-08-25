
//
// An object group distribution is a distribution over a group of objects, trained on a set of the same.
// A group of objects is a set of model-size-rotation triplets. It does not suggest any information about
// the support hierarchy of the objects.
//

struct Model;
struct DisembodiedObject;
struct CategoryDatabase;

class ObjectGroupDistribution
{
public:
    virtual vector<DisembodiedObject> sample() const = 0;
    virtual double probabilityCategoryCountAtLeast(const string &categoryName, int categoryCount) const = 0;
    virtual set<string> categoryList() const = 0;
    virtual void output(HTMLExporter &e, const string &name) const = 0;
};

class ObjectGroupDistributionMemorizer : public ObjectGroupDistribution
{
public:
    ObjectGroupDistributionMemorizer(const vector< vector<DisembodiedObject> > &samples);
    double probabilityCategoryCountAtLeast(const string &categoryName, int categoryCount) const;

    vector<DisembodiedObject> sample() const
    {
        return ml::util::randomElement(_samples);
    }

    set<string> categoryList() const
    {
        return _categoryList;
    }

    void output(HTMLExporter &e, const string &name) const;

private:
    vector< vector<DisembodiedObject> > _samples;
    set<string> _categoryList;
};

class ObjectGroupDistributionIndependent : public ObjectGroupDistribution
{
public:
    ObjectGroupDistributionIndependent(const vector< vector<DisembodiedObject> > &samples, const CategoryDatabase &categories);
    vector<DisembodiedObject> sample() const;
    double probabilityCategoryCountAtLeast(const string &categoryName, int categoryCount) const;
    set<string> categoryList() const
    {
        return _categoryList;
    }

    void output(HTMLExporter &e, const string &name) const;

private:
    //
    // for each category, stores the number of occurences of a given total count in every scene.
    // For example, 4, 4, 0, 0, 0 would indicate 2 scenes where 4 instances occured and 3 scenes where 0 instances ocured.
    //
    map<string, vector<int> > _categoryFrequencies;

    const CategoryDatabase *_categories;
    set<string> _categoryList;

    //
    // samples are not needed, except for debugging / analysis purposes.
    //
    vector< vector<DisembodiedObject> > _samples;
};