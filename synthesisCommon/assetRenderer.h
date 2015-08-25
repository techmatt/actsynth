
struct AssetRendererConstantBuffer
{
    mat4f worldViewProj;
    ml::vec4f modelColor;
};

struct AssetRendererConstantBufferSH
{
  float lightingCoefficients[9];
  ml::vec3f dummy;  //typically these buffers should be aligned to 16 bytes
};

struct LightingParameters
{
    LightingParameters()
    {
        randomize();
    }
    void randomize();

    AssetRendererConstantBufferSH sh;
};

class AssetRenderer
{
public:
    void init(ml::D3D11GraphicsDevice &g);
    void renderObjects(AssetManager &assets, ml::GraphicsDevice &g, const mat4f &cameraPerspective, const Scene &s, const vector<vec3f> &objectColors, bool renderSimplifiedModel);
    void renderObject(AssetManager &assets, ml::GraphicsDevice &g, const mat4f &cameraPerspective, const ObjectInstance &m, const vec3f &color, bool renderSimplifiedModel);
    void renderModel(AssetManager &assets, ml::GraphicsDevice &g, const mat4f &cameraPerspective, const string &modelId, const mat4f &modelToWorld, const vec3f &color, bool renderSimplifiedModel);

    void renderPBRTObjects(AssetManager &assets, ml::GraphicsDevice &g, const string& filename, const Cameraf &camera, const Scene &s, const vector<vec3f> &objectColors);
    void renderPBRTModel(AssetManager &assets, ml::GraphicsDevice &g, const string& filename, const Cameraf &camera, const string &modelId, const mat4f &modelToWorld, const vec3f &color);

    void renderPOVRayObjects(AssetManager &assets, ml::GraphicsDevice &g, const string& filename, const Cameraf &camera, const Scene &s, const vector<vec3f> &objectColors);
    void renderPOVRayModel(AssetManager &assets, ml::GraphicsDevice &g, const string& filename, const Cameraf &camera, const string &modelId, const mat4f &modelToWorld, const vec3f &color);

    void renderAgent(ml::GraphicsDevice &g, const mat4f &cameraPerspective, const Agent &a, const vec3f &color);
    void renderMesh(ml::GraphicsDevice &g, const mat4f &cameraPerspective, const ml::D3D11TriMesh &mesh, const vec3f &color = vec3f(1.0f, 1.0f, 1.0f));
    void renderMesh(ml::GraphicsDevice &g, const mat4f &cameraPerspective, const mat4f &meshToWorld, const ml::D3D11TriMesh &mesh, const vec3f &color = vec3f(1.0f, 1.0f, 1.0f));

    void renderCylinder(ml::GraphicsDevice &g, const mat4f &cameraPerspective, const vec3f &p0, const vec3f &p1, const vec3f &color);
    void renderSphere(ml::GraphicsDevice &g, const mat4f &cameraPerspective, const vec3f &center, float radius, const vec3f &color);
    void renderBox(ml::GraphicsDevice &g, const mat4f &cameraPerspective, const vec3f &center, float radius, const vec3f &color);

    void renderPolygon(ml::GraphicsDevice &g, const mat4f &cameraPerspective, const Polygonf &poly, const vec3f &color, float height);

    const D3D11TriMesh& sphereMesh() const
    {
        return m_sphere;
    }

    const D3D11TriMesh& boxMesh() const
    {
        return m_box;
    }

    const D3D11TriMesh& agentHead() const
    {
        return m_agentHead;
    }

    LightingParameters& lighting()
    {
        return m_lighting;
    }

private:
    D3D11ShaderManager m_shaders;
    ml::D3D11ConstantBuffer<AssetRendererConstantBuffer> m_constants;
    ml::D3D11ConstantBuffer<AssetRendererConstantBufferSH> m_constantsSH;


    D3D11TriMesh m_agentHead;
    D3D11TriMesh m_agentLook;
    D3D11TriMesh m_agentBase;

    D3D11TriMesh m_cylinder;
    D3D11TriMesh m_sphere;
    D3D11TriMesh m_box;
    LightingParameters m_lighting;
};
