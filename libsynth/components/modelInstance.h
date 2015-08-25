
struct Model;
struct Scene;

enum ContactType
{
    ContactUpright = 0, //Z-dot-normal = 0
    ContactWall = 1, //Z-dot-normal = 1
    ContactOther = 2
};

inline string contactTypeToString(ContactType c)
{
    if (c == ContactUpright) return "upright";
    if (c == ContactWall) return "wall";
    if (c == ContactOther) return "other";
    return "unknown";
}

//
// A DisembodiedObject is a model that has size and upright orientation, but has not been localized in an environment.
//
struct DisembodiedObject
{
    DisembodiedObject()
    {
        model = NULL;
    }

    mat4f makeBaseTransformCentered() const;
    mat4f makeBaseTransform(float elevation = 0.003f) const;

	OBBf computeWorldBBox() const;
    
    static void output(HTMLExporter &e, const string &name, const vector< vector<DisembodiedObject> > &samples);
    
    string modelId;
    Model *model;

    //
    // modelNormal and modelDown are unit-length vectors in an origin-centered model-space coordinate frame.
    // For upright objects, modelNormal is the vector which maps to the normal direction and modelDown is ignored.
    // For wall-mounted objects, modelNormal is the outward direction, and modelDown is the direction that should point toward the negative Z direction.
    //
    ContactType contactType;
    vec3f modelNormal;
    vec3f modelDown;

    float scale;
    float distanceToWall;
    float coordianteFrameRotation;

    vec2f agentFace;

    //
    // should DisembodiedObject store the parent object category, and possibly information about the parent contact surface?
    //
};

//
// An ObjectInstance is a DisembodiedObject that has been localized to a specific location in a scene.
//
struct ObjectInstance
{
    ObjectInstance()
    {
        scene = nullptr;
    }
    ObjectInstance(Scene *_scene)
    {
        scene = _scene;
    }

    void output(HTMLExporter &e, const string &name) const;
    static void output(HTMLExporter &e, const string &name, const vector<const ObjectInstance*> &instances);
    static void output(HTMLExporter &e, const string &name, const vector<ObjectInstance> &instances);

    string categoryName() const;
    string parentCategoryName() const;

    const Scene *scene;

    DisembodiedObject object;
    
    mat4f modelToWorld;

    // local coordinate frame of the object instance.
    // this is not correctly saved for all SceneStudio files so is not recorded
    //vec3f cU, cV, cNormal;
    
    //
    // -1 indicates no parent (the base architecture)
    //
    int parentObjectInstanceIndex;

    int parentMeshIndex;
    int parentTriIndex;
    vec2f parentUV;

    //
    // these are not stored in the SceneStudio scene format, but require the mesh to be loaded. They are computed in a preprocess
    // and saved in a cache, which is then loaded immediately after scenes are loaded from the WSS JSON file.
    //
    vec3f contactPosition;
    vec3f contactNormal;
	OBBf worldBBox;
};
