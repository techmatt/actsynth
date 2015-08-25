/*
#include "libsynth.h"


void ActivityCluster::output(const std::string &filename) const
{
    HTMLExporter exporter(filename, "Activity cluster: " + id);

    objectGroupDistribution->output(exporter, "Object group distribution");

    agentDistribution.output(exporter, "Agent group distribution");

    exporter.appendHeader1("Category distributions");
    for (const auto &p : categoryDistributions)
        p.second.output(exporter);

}
*/