#include "libsynth.h"

void ScanDatabase::load(const string &scanDirectory)
{
    Directory dir(scanDirectory);

    vector<string> scansToLoad;
    
    if (synthParams().scanName == "all")
    {
        for (const string &s : ml::util::getFileLines(synthParams().synthesisDir + "CNNScans.txt"))
        {
            scansToLoad.push_back(s);
        }
    }
    else
    {
        scansToLoad.push_back(synthParams().scanName);
    }

    for (const string &s : dir.filesWithSuffix(".ply"))
    {
        //
        // for faster loading, load only the one scan we need.
        //
        string name = ml::util::remove(s, ".ply");
        if (ml::util::contains(scansToLoad, name))
        {
            Scan &scan = scans[name];
            scan.name = name;

            const string meshFilename = scanDirectory + scan.name + ".ply";
            const string alignmentFilename = scanDirectory + scan.name + "_align.txt";
            const string agentFilename = scanDirectory + scan.name + "_agents" + synthParams().scanAgentSuffix + ".arv";

            scan.load(meshFilename, alignmentFilename, agentFilename);
        }
    }
}
