/*

ActivityCluster should no longer be needed.

struct ActivityCluster
{
    void output(const std::string &filename) const;

    std::string id;

    //
    // a distribution over the collection of objects that are associated with this activity
    //
    ObjectGroupDistribution *objectGroupDistribution;

    std::map<std::string, ActivityCategoryDistributions> categoryDistributions;

    //
    // each activity cluster "spawns" a number of child agents. Each child agent stores its offset
    // and support object. If the support object is produced by the ActivityCluster (ex. a sofa),
    // then the agent goes directly on the ActivityCluster's instance of that object. Otherwise,
    // a new support object is produced for each agent (ex. chairs around a dining table.)
    //
    AgentGroupDistributionMemorizer agentDistribution;
};
*/