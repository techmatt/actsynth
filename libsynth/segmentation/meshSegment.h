
struct MeshSegment
{
    MeshSegment()
    {
        mesh = NULL;
    }
    MeshSegment(const TriMeshf& _mesh, const vector<UINT>& _vtxIndices, const UINT _id)
    {
        mesh = &_mesh;
        vertexIndices = _vtxIndices;
        id = _id;
        init();
    }

    void init();

	static OBBf computeUpOrientedOBB(const vector<vec3f> &points);

    const TriMeshf* mesh;
    UINT id;

    vector<UINT> vertexIndices;
    vector<vec3f> points;

    bbox3f aabb;
	OBBf oobb;
};
