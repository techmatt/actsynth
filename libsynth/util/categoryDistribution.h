
//
// A category distribution is a weighted set of categories
//
class CategoryDistribution
{
public:
    CategoryDistribution() {}
    CategoryDistribution(const vector<string> &instances)
    {
        for (const string &i : instances)
            categories[i]++;

        for (auto &p : categories)
            p.second /= (double)instances.size();
    }

    void output(HTMLExporter &e, const string &name) const
    {
        vector<string> headers;
        headers.push_back("category");
        headers.push_back("frequency");

        vector<string> entries;
        for (const auto &p : categories)
        {
            entries.push_back(p.first);
            entries.push_back(to_string(p.second));
        }

        e.appendTable(name, headers, entries);
    }

    string sample() const
    {
        double sum = 0.0;
        for (const auto &c : categories)
            sum += c.second;

        double x = ml::util::randomUniform(0.0, sum);
        for (const auto &c : categories)
        {
            x -= c.second;
            if (x <= 0.0)
                return c.first;
        }

        cout << "sum failed in CategoryDistribution::sample()" << endl;
        return categories.begin()->first;
    }

    map<string, double> categories;
};
