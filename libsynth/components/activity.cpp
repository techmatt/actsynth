
#include "libsynth.h"

void ActivityCategoryDistributions::output(HTMLExporter & e) const
{
    e.appendHeader2(category + " distributions");
    
    auto outputDistribution = [&](HTMLExporter &e, Distribution3D *dist, const string &name)
    {
        if (dist == nullptr)
            e.appendText("No " + name + " distribution");
        else
            dist->output(e, name);
    };

    outputDistribution(e, gaze, "gaze");
    outputDistribution(e, palm, "palm");
    outputDistribution(e, fingertip, "fingertip");
    outputDistribution(e, backSupport, "backSupport");
    outputDistribution(e, base, "base");
    outputDistribution(e, center, "center");
}

void Activity::output(const string &filename) const
{
    HTMLExporter exporter(filename, "Activity: " + id);

    supportDistribution.output(exporter, "support distribution");
    objectGroupDistribution->output(exporter, "object group distribution");

    exporter.appendHeader1("Category distributions");
    for (const auto &p : categoryDistributions)
        p.second.output(exporter);
}
