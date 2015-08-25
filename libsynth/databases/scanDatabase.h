
struct ScanDatabase
{
	void load(const string &scanDirectory);
    
    const Scan& getScan(const string &scanName) const
    {
        if (scans.count(scanName) == 0)
        {
            cout << "scan not found: " << scanName << endl;
            return scans.begin()->second;
        }
        return scans.find(scanName)->second;
    }

    Scan& getScan(const string &scanName)
    {
        if (scans.count(scanName) == 0)
        {
            cout << "scan not found: " << scanName << endl;
            return scans.begin()->second;
        }
        return scans.find(scanName)->second;
    }

    vector<string> scanNames() const
    {
        vector<string> result;
        for (const auto &s : scans)
            result.push_back( ml::util::fileNameFromPath(s.second.pathName) );
        return result;
    }

    map<string, Scan> scans;
};
