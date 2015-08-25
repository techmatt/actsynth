
struct InteractionMapEntry
{
    static RGBColor interactionColor(const string &s)
    {
        if (s == "gaze") return RGBColor(0, 162, 232);
        if (s == "feet") return RGBColor(181, 230, 29);
        if (s == "fingertip") return RGBColor(34, 177, 76);
        if (s == "backSupport") return RGBColor(185, 122, 87);
        if (s == "hips") return RGBColor(237, 28, 36);
        return RGBColor::White;
    }

    RGBColor interactionColor() const
    {
        return interactionColor(type);
    }

    struct Centroid
    {
        vec3f pos;

        int meshIndex;
        int triangleIndex;
        ml::vec2f uv;
    };

    //
    // interaction types:
    // gaze
    // feet
    // fingertip
    // backSupport
    // hips
    //
    string type;

    //
    // A small set of important regions. Used to avoid iterating over all faces.
    //
    vector<Centroid> centroids;

    //
    // like models, faceValues is treated as a cache are is not all loaded at once. They may be emptied at any time but should always have an entry.
    // the centroids should be all that is required for synthesis.
    // access via faceValues[meshIndex][triangleIndex]
    //
    vector< vector<float> > faceValues;
};

struct InteractionMap
{
    void save(const string &filename) const;
    void load(const string &filename);

    map<string, InteractionMapEntry> entries;
};
