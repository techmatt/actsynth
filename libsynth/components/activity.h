
//
// many of these distributions don't really matter; most of what matters is the accessibility.  These distributions are still used to
// sample the initial placement of objects, but only one is likely to be needed for this (ex. base or center).
//
struct ActivityCategoryDistributions
{
    ActivityCategoryDistributions()
    {
        category = "uninitialized";
    }

    ActivityCategoryDistributions(const string &_category)
    {
        category = _category;
        gaze = NULL;
        palm = NULL;
        fingertip = NULL;
        backSupport = NULL;
        hips = NULL;
        base = NULL;
        center = NULL;
    }

    void output(HTMLExporter & e) const;

    static vector<string> distributionList()
    {
        vector<string> result = { "gaze", "palm", "fingertip", "backSupport", "hips", "base", "center" };
        return result;
    }

    Distribution3D*& getDistribution(const string &name)
    {
        if (name == "gaze") return gaze;
        if (name == "palm") return palm;
        if (name == "fingertip") return fingertip;
        if (name == "backSupport") return backSupport;
        if (name == "hips") return hips;
        if (name == "base") return base;
        if (name == "center") return center;
        return gaze;
    }

    const Distribution3D* getDistribution(const string &name) const
    {
        if (name == "gaze") return gaze;
        if (name == "palm") return palm;
        if (name == "fingertip") return fingertip;
        if (name == "backSupport") return backSupport;
        if (name == "hips") return hips;
        if (name == "base") return base;
        if (name == "center") return center;
        return NULL;
    }

    string category;
    
    // distribution of the centroids of the gaze interaction in the agent coordinate frame
    Distribution3D *gaze;

    // distribution of the centroids of the palm interaction in the agent coordinate frame
    Distribution3D *palm;

    // distribution of the centroids of the fingertip interaction in the agent coordinate frame
    Distribution3D *fingertip;

    // distribution of the centroids of the backSupport interaction in the agent coordinate frame
    Distribution3D *backSupport;

    // distribution of the centroids of the hips interaction in the agent coordinate frame
    Distribution3D *hips;

    // distribution of the base of the object in the agent coordinate frame
    Distribution3D *base;

    // distribution of the OOBB center of the object in the agent coordinate frame
    Distribution3D *center;

    //
    // When the agent is supposed to be sitting, the agent is always explicitly placed on the sittable region
    // and the hips distribution is probably not needed.
    //
};

struct Activity
{
    void output(const string &filename) const;

    static vector<string> activityList()
    {
        vector<string> result;
        result.push_back("use desktop PC");
        result.push_back("living room");
        result.push_back("dining table");
        result.push_back("bed");
        return result;
    }

    static map<string, vec3f> activityColors()
    {
        map<string, vec3f> result;
        result["use desktop PC"] = vec3f(0.8f, 0.25f, 0.25f);
        result["living room"]    = vec3f(0.8f, 0.8f, 0.25f);
        result["dining table"]   = vec3f(0.25f, 0.25f, 0.8f);
        result["bed"]            = vec3f(0.25f, 0.8f, 0.25f);
        return result;
    }

    string id;

    //
    // regions of space that need to be free for the agent
    // specified in the coordinate frame where the origin is the floor beneath the agent's feet, and X is the agent's front vector.
    //
    vector<bbox3f> agentFreeSpaceBlockers;

    //
    // the weighted set of categories that have been observed beneath the person performing this activity
    //
    CategoryDistribution supportDistribution;

    //
    // a distribution over the collection of objects that are associated with this activity
    //
    ObjectGroupDistribution *objectGroupDistribution;

    map<string, ActivityCategoryDistributions> categoryDistributions;
};
