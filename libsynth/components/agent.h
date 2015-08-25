
struct Scene;

struct Agent
{
    void output(HTMLExporter &e) const;

    //bool isActivityClusterInstance() const
    //{
    //    return ml::util::endsWith(activity, "Instance");
    //}

    //bool isActivityCluster() const
    //{
    //    return ml::util::endsWith(activity, "Cluster");
    //}

    vector<const ObjectInstance*> boundObjects() const;

    const ObjectInstance& supportObject() const;

    vec3f handPos() const
    {
        return headPos - vec3f::eZ * 0.25f;
    }

    vec3f spinePos() const
    {
        return headPos - vec3f::eZ * 0.35f;
    }

    vec3f worldToAgentBasis(const vec3f &worldPos) const
    {
        vec3f result;
        vec3f diff = worldPos - headPos;

        // x = gaze component
        // y = right component
        // z = up component
        result.x = vec3f::dot(diff, gazeDir);
        result.y = vec3f::dot(diff, rightDir);
        result.z = vec3f::dot(diff, vec3f::eZ);
		return result;
    }

    //
    // coordinate frame of the agent
    //
    vec3f headPos;
    vec3f gazeDir;
    vec3f rightDir;

    //
    // position in the scene supporting the agent.
    // Typically, this is the intersection of the (x,y) ray with the scene.
    //
    vec3f supportPos;

    //
    // Agents can also represent activityClusters, which are denoted by the id.
    // Example: diningTableCluster is the ActivityCluster composed of diningTableInstance activities.
    //
    string activity;

    Scene *scene;

    //
    // index of the ModelInstance in scene that supports this agent. 
    //
    unsigned int supportObjectIndex;

    //
    // set of model instances that are bound to the activity this agent is performing. Does not include the supporting object.
    //
    vector<unsigned int> boundObjectIndices;
};
