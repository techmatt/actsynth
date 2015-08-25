
//
// An agent group distribution is a distribution over a group of agents, trained on a set of the same.
// It is associated with an ActivityCluster. For example, it might be the distribution of agents around
// a dining table.
//

struct AgentGroupEntry
{
    static void output(HTMLExporter &e, const string &name, const vector< vector<AgentGroupEntry> > &samples);

    string activity;

    //
    // information about the object supporting this agent
    //
    DisembodiedObject supportObject;

    //
    // offset of this agent from the centroid of the activity cluster
    //
    ml::vec2f offset;
};

class AgentGroupDistributionMemorizer
{
public:
    AgentGroupDistributionMemorizer() {}
    AgentGroupDistributionMemorizer(const vector< vector<AgentGroupEntry> > &samples)
    {
        _samples = samples;
    }

    vector<AgentGroupEntry> sample() const
    {
        return ml::util::randomElement(_samples);
    }

    void output(HTMLExporter &e, const string &name) const;

private:
    vector< vector<AgentGroupEntry> > _samples;
};
