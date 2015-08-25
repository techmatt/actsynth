
#include "libsynth.h"

void AgentGroupEntry::output(HTMLExporter &e, const string &name, const vector< vector<AgentGroupEntry> > &samples)
{
    e.appendHeader1(name);
    for (auto &s : samples)
    {
        e.file << "<p>";
        for (auto &a : s)
        {
            e.file << "<b>" << a.activity << "</b>-" << a.supportObject.model->categoryName << "-" << a.offset.x << "," << a.offset.y << " | ";
        }
        e.file << "</p>" << endl;
    }
    e.appendLineBreak();
}

void AgentGroupDistributionMemorizer::output(HTMLExporter &e, const string &name) const
{
    AgentGroupEntry::output(e, name + " samples", _samples);
}
