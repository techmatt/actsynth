
struct Category;

struct Model
{
    Model()
    {
        //categoryPtr = NULL;
        blacklisted = false;
    }

    string imageURL() const;
    string imageHTML(unsigned int width = 128, unsigned int height = 128) const;

    bool blacklisted;
    string id;
    string categoryName;
    
    ml::bbox3f bbox;

    //
    // mesh data is loaded lazily -- it will be empty until loaded from the cache via loadMesh
    // currently, meshes is not used, it is loaded as a D3D11 mesh in assets.
    //
    //vector<ml::TriMeshf> meshes;
    //void loadMesh();
};
