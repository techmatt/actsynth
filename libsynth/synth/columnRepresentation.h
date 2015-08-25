
struct SceneColumn
{
    float residueSize() const
    {
        return maxAnyZ - maxSupportZ;
    }

    //vec2f center;

    //
    // here we could do either a general version storing every plane, or
    // a faster and simpler version storing only the plane at maximum height.
    //
    float maxSupportZ;
    float maxAnyZ;
};

class GeometricRepresentation;
struct ModelAssetData;

struct ColumnRepresentation
{
    //
    // returns a score that compares two column representations. A high score indicates more similar scenes.
    //
    static double score(const ColumnRepresentation &a, const ColumnRepresentation &b);

    static ColumnRepresentation constructFromScan(const Scan &s, float _columnDimension, const GeometricRepresentation &geometry);

    void reset(const ColumnRepresentation &base);

    void includeObjectBBoxSimplification(const OBBf &obb, bool isSupport);
    void includeObjectRayTracing(const TriMeshRayAcceleratorf &accel, const bbox3f &worldBBox, const mat4f &modelToWorld, const mat4f &worldToModel, bool isSupport);
	void includeObjectMaxZMap(const ml::Grid2f& maxZMap, const mat4f &modelToWorld, const mat4f &worldToModel, const mat4f& modelToVoxel, bool isSupport);
    //static ColumnRepresentation constructFromTriangles(const ColumnRepresentation &base, const vector<Trianglef> &supportTriangles, const vector<Trianglef> &residueTriangles);

    vec2f getColumnCenter(int x, int y) const
    {
        return bbox.getMin() + vec2f(x * columnDimension, y * columnDimension);
    }

    float columnDimension;
    ml::bbox2f bbox;
    ml::Grid2<SceneColumn> columns;

private:
    float findHighestInteresction(const vec2f &center, const vector< pair<const TriMeshRayAcceleratorf*, ml::mat4f> > &accel, const mat4f &modelToWorld, const mat4f &worldToModel)
    {
        Rayf ray(vec3f(center, 100.0f), -vec3f::eZ);

        UINT objectIndex;
        auto intersection = TriMeshRayAcceleratorf::getFirstIntersectionTransform(ray, accel, objectIndex);
        if (intersection.isValid())
        {
            vec3f intersect = modelToWorld.transformAffine(intersection.getSurfacePosition());
            return std::max(0.0f, intersect.z);
        }
    
        return 0.0f;
    }

    float findHighestInteresction(const vec2f &center, const Trianglef &tri)
    {
        float maxZ = 0.0f;
        Rayf ray(vec3f(center, 100.0f), -vec3f::eZ);
        
        float t, u, v;
        if (ml::intersection::intersectRayTriangle(tri, ray, t, u, v))
        {
            vec3f intersect = ray.getHitPoint(t);
            maxZ = std::max(maxZ, intersect.z);
        }
        
        return maxZ;
    }

    /*float findHighestInteresction(const vec2f &center, const vector<Trianglef> &triangles)
    {
        float maxZ = 100.0f;
        Rayf ray(vec3f(center, maxZ), -vec3f::eZ);
        for (const Trianglef &tri : triangles)
        {
            float t, u, v;
            if (ml::intersection::intersectRayTriangle(tri, ray, t, u, v))
            {
                vec3f intersect = ray.getHitPoint(t);
                maxZ = std::max(maxZ, intersect.z);
            }
        }
        return maxZ;
    };*/
};
