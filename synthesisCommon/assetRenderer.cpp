#include "main.h"

#include <boost/filesystem.hpp>

void LightingParameters::randomize()
{
    auto r = [](float a, float b) { return ml::util::randomUniform(a, b); };
    
    float sum = 0.0f;
    for (int i = 0; i < 9; i++)
    {
        sh.lightingCoefficients[i] = (float)ml::math::clamp(ml::RNG::global.normal(0.8, 0.5), 0.1, 1.5);
        sum += sh.lightingCoefficients[i];
    }

    float targetSum = r(2.0f, 5.0f);
    for (int i = 0; i < 9; i++)
    {
        sh.lightingCoefficients[i] = std::max(sh.lightingCoefficients[i] / sum * targetSum, 0.6f);
    }
}

void AssetRenderer::init(ml::D3D11GraphicsDevice &g)
{
    m_shaders.init(g);
    m_shaders.registerShader("../../shaders/modelRendererColor.shader", "basicColor");
    m_shaders.registerShader("../../shaders/modelRendererTexture.shader", "basicTexture");
	m_shaders.registerShader("../../shaders/modelRendererSH.hlsl", "basicSH", "vertexShaderMain", "vs_4_0", "pixelShaderMain", "ps_4_0");
	std::vector<std::pair<std::string, std::string>> shaderMacrosTexture;
	shaderMacrosTexture.push_back(std::make_pair("TEXTURE", "1"));
    m_shaders.registerShader("../../shaders/modelRendererSH.hlsl", "basicSHTexture", "vertexShaderMain", "vs_4_0", "pixelShaderMain", "ps_4_0", shaderMacrosTexture);
    m_constants.init(g);
    m_constantsSH.init(g);

    m_agentHead.load(g, ml::Shapesf::sphere(0.15f, vec3f::origin));
    m_agentLook.load(g, ml::Shapesf::cylinder(0.02f, 0.35f, 2, 15, ml::vec4f(1.0f, 1.0f, 1.0f, 1.0f)));
    m_agentBase.load(g, ml::Shapesf::torus(vec3f::origin, 0.15f, 0.05f, 15, 15));

    m_sphere.load(g, ml::Shapesf::sphere(1.0f, vec3f::origin));
    m_cylinder.load(g, ml::Shapesf::cylinder(0.01f, 1.0f, 2, 15, ml::vec4f(1.0f, 1.0f, 1.0f, 1.0f)));
    m_box.load(g, ml::Shapesf::box(1.0f));
}

void AssetRenderer::renderObjects(AssetManager &assets, ml::GraphicsDevice &g, const mat4f &cameraPerspective, const Scene &s, const vector<vec3f> &objectColors, bool renderSimplifiedModel)
{
    for (UINT objectIndex = 0; objectIndex < s.objects.size(); objectIndex++)
    {
        renderObject(assets, g, cameraPerspective, s.objects[objectIndex], objectColors[objectIndex], renderSimplifiedModel);
    }
}

void AssetRenderer::renderAgent(ml::GraphicsDevice &_g, const mat4f &cameraPerspective, const Agent &a, const vec3f &color)
{
    ml::D3D11GraphicsDevice &g = _g.castD3D11();
    g.setCullMode(D3D11_CULL_NONE);

    m_shaders.bindShaders("basicColor");

    AssetRendererConstantBuffer constants;

    constants.worldViewProj = cameraPerspective * mat4f::translation(a.headPos);
    constants.modelColor = ml::vec4f(color, 1.0f);
    m_constants.updateAndBind(constants, 0);
    m_agentHead.render();

    constants.worldViewProj = cameraPerspective * mat4f::translation(a.supportPos);
    constants.modelColor = ml::vec4f(color * 0.9f, 1.0f);
    m_constants.updateAndBind(constants, 0);
    m_agentBase.render();

    constants.worldViewProj = cameraPerspective * mat4f::translation(a.headPos) * mat4f::face(vec3f::eZ, a.gazeDir);
    constants.modelColor = ml::vec4f(color * 0.8f, 1.0f);
    m_constants.updateAndBind(constants, 0);
    m_agentLook.render();

    g.setCullMode(D3D11_CULL_FRONT);
}

void AssetRenderer::renderObject(AssetManager &assets, ml::GraphicsDevice &g, const mat4f &cameraPerspective, const ObjectInstance &m, const vec3f &color, bool renderSimplifiedModel)
{
    renderModel(assets, g, cameraPerspective, m.object.modelId, m.modelToWorld, color, renderSimplifiedModel);

    /*if (m.object.model->categoryName->isHandOrientation)
    {
    const vec3f center = m.object.model->bbox.getCenter();
    const vec3f p0 = m.modelToWorld * center;

    vec3f p1 = m.modelToWorld * (center + vec3f(m.object.agentFace.x, m.object.agentFace.y, 0.0f));
    p1 = (p1 - p0).getNormalized() * 0.35f + p0;

    renderCylinder(g, cameraPerspective, p0, p1, vec3f(0.9f, 0.9f, 0.1f));
    }*/
}

void AssetRenderer::renderModel(AssetManager &assets, ml::GraphicsDevice &g, const mat4f &cameraPerspective, const string &modelId, const mat4f &modelToWorld, const vec3f &color, bool renderSimplifiedModel)
{
    g.castD3D11().setCullMode(D3D11_CULL_FRONT);

	//TODO add some randomized coefficients here
    //AssetRendererConstantBufferSH constantsSH;
    //for (unsigned int i = 0; i < 9; i++)
    //  constantsSH.lightingCoefficients[i] = 3.0f/9.0f;

    m_constantsSH.updateAndBind(m_lighting.sh, 1);

    const ModelAssetData &ModelAssetData = assets.loadModel(g, modelId);
    for (const ModelMeshData &meshData : ModelAssetData.data)
    {
        AssetRendererConstantBuffer constants;
        constants.worldViewProj = cameraPerspective * modelToWorld;
        constants.modelColor = ml::vec4f(color, 1.0f);

        if (meshData.texture != NULL)
        {
           //m_shaders.bindShaders("basicTexture");
            m_shaders.bindShaders("basicSHTexture");
            
            
            constants.modelColor = vec4f(color, 1.0f);
            meshData.texture->texture.bind();
        }
        else
        {
            // m_shaders.bindShaders("basicColor");
            m_shaders.bindShaders("basicSH");


            constants.modelColor = meshData.material.m_diffuse;
            if (meshData.material.m_diffuse.lengthSq() <= 0.01)
                constants.modelColor = vec4f(0.4f, 0.4f, 0.4f, 1.0f);

            constants.modelColor = vec4f(std::min(color.x, meshData.material.m_diffuse.x),
                std::min(color.y, meshData.material.m_diffuse.y),
                std::min(color.z, meshData.material.m_diffuse.z), 1.0f);

            if (color.lengthSq() >= 3.1f)
                constants.modelColor = vec4f(color, 1.0f);
        }
        m_constants.updateAndBind(constants, 0);

        if (renderSimplifiedModel)
            meshData.meshSimplified.render();
        else
            meshData.meshOriginal.render();

    }

    g.castD3D11().setCullMode(D3D11_CULL_NONE);

}

void AssetRenderer::renderPBRTObjects(AssetManager &assets, ml::GraphicsDevice &g, const string& filename, const Cameraf &camera, const Scene &s, const vector<vec3f> &objectColors)
{
    boost::filesystem::path path(filename);
    std::string path_models = path.parent_path().string() + "/models";
    if (!boost::filesystem::exists(path_models))
        boost::filesystem::create_directory(path_models);
    std::ofstream file(filename, std::ofstream::out | std::ofstream::app);
    for (UINT objectIndex = 0; objectIndex < s.objects.size(); objectIndex++)
    {
        //
        // Objects with negative colors are a signal they should be skipped
        //
        if (objectColors[objectIndex].x >= 0.0f)
        {
            const std::string filename_object = s.objects[objectIndex].object.modelId + "_" + s.id + "_" + to_string(objectIndex) + ".pbrt";
            file << "Include \"models/" << filename_object << "\"" << endl;
            renderPBRTModel(assets, g, path_models + "/" + filename_object, camera, s.objects[objectIndex].object.modelId, s.objects[objectIndex].modelToWorld, objectColors[objectIndex]);
        }

    }
    file.close();

    return;
}

void AssetRenderer::renderPBRTModel(AssetManager &assets, ml::GraphicsDevice &g, const string& filename, const Cameraf &camera, const string &modelId, const mat4f &modelToWorld, const vec3f &color)
{
    boost::filesystem::path path(filename);
    std::string directory = path.parent_path().parent_path().string() + "/";
    std::string directory_texturesRAW = directory + "/texturesRAW/";
    if (!boost::filesystem::exists(directory_texturesRAW))
        boost::filesystem::create_directory(directory_texturesRAW);
    std::string directory_texturesEXR = directory + "/texturesEXR/";
    if (!boost::filesystem::exists(directory_texturesEXR))
        boost::filesystem::create_directory(directory_texturesEXR);
    std::string filename_stem = path.stem().string();

    std::ofstream file(filename);

    const mat4f modelToCamera = camera.getCamera() * modelToWorld;

    const ModelAssetData &ModelAssetData = assets.loadModel(g, modelId);
    int empty_texture_name_count = 0;
    for (const ModelMeshData &meshData : ModelAssetData.data)
    {
        std::string localTextureName, localTextureFilename;
        if (meshData.texture != NULL)
        {
            std::string textureName = boost::filesystem::path(meshData.texture->name).stem().string();
            if (textureName.empty()) {
                textureName = ml::util::zeroPad(empty_texture_name_count++, 4);
            }
            const std::string textureFilenameRAW = directory_texturesRAW + "/" + textureName + ".raw";
            const std::string textureFilenameEXR = directory_texturesEXR + "/" + textureName + ".exr";

            localTextureName = filename_stem + "_" + textureName;
            localTextureFilename = textureName;

            if (!ml::util::fileExists(textureFilenameEXR))
            {
				Bitmap bmp = meshData.texture->texture.getImage();
                UINT width = (UINT)bmp.getWidth(), height = (UINT)bmp.getHeight();

                std::ofstream textureFile(textureFilenameRAW, std::ios::binary);
                textureFile.write((const char*)(&width), sizeof(UINT));
                textureFile.write((const char*)(&height), sizeof(UINT));

                std::vector<ml::vec3f> rawData(width * height);
                for (UINT y = 0; y < height; y++)
                {
                    for (UINT x = 0; x < width; x++)
                    {
                        ml::vec3f v = ml::vec3f(RGBColor(bmp(x, y)));
                        rawData[(height - 1 - y) * width + x] = v;
                    }
                }
                textureFile.write((const char*)(&rawData[0]), sizeof(float) * 3 * width * height);
                textureFile.close();

                const std::string sysCommand = "V:\\data\\synthesis\\pbrt\\bin\\tifftoexr.exe \"" + textureFilenameRAW + "\" \"" + textureFilenameEXR + "\"";
                system(sysCommand.c_str());
            }
        }

        std::vector<UINT> frontFacingIndices;
        const auto &vertices = meshData.meshOriginal.getTriMesh().getVertices();
        for (const vec3ui& index : meshData.meshOriginal.getTriMesh().getIndices())
        {
            const UINT i0 = index[0];
            const UINT i1 = index[1];
            const UINT i2 = index[2];

            const vec3f v0 = modelToCamera.transformAffine(vertices[i0].position);
            const vec3f v1 = modelToCamera.transformAffine(vertices[i1].position);
            const vec3f v2 = modelToCamera.transformAffine(vertices[i2].position);
            vec3f normal = ml::math::triangleNormal(v0, v1, v2);

            bool frontFacing = normal.z >= 0.0f;

            if (frontFacing)
            {
                frontFacingIndices.push_back(i0);
                frontFacingIndices.push_back(i1);
                frontFacingIndices.push_back(i2);
            }
        }

        bool tintedColor = (vec3f::distSq(color, vec3f(1.0f, 1.0f, 1.0f)) > 0.0f);

        if (frontFacingIndices.size() > 0)
        {
            //if (meshData.material.m_shiny > 0)
            //{
            //file << "Material \"glass\"" << endl;
            //file << "  \"color Kt\" [" << curGeometry.diffuse.x << ' ' << curGeometry.diffuse.y << ' ' << curGeometry.diffuse.z << "]" << endl;
            //file << "  \"color Kr\" [" << curGeometry.diffuse.x << ' ' << curGeometry.diffuse.y << ' ' << curGeometry.diffuse.z << "]" << endl;
            //}
            //else
            if (meshData.texture == NULL || tintedColor)
            {
                ml::vec3f diffuse = meshData.material.m_diffuse.getVec3();
                if (tintedColor)
                    diffuse = ml::math::max(diffuse, vec3f(0.25f, 0.25f, 0.25f));

                file << "Material \"matte\" \"color Kd\" [" << diffuse.x * color.x << ' ' << diffuse.y * color.y << ' ' << diffuse.z * color.z << "]" << endl;
            }
            else
            {
                file << "Texture \"" << localTextureName << "\" \"color\" \"imagemap\" \"string filename\" \"texturesEXR/" << localTextureFilename << ".exr\"" << endl;
                //file << "Texture \"" << localTextureName << "_s\" \"color\" \"scale\" \"texture tex1\" \"" << localTextureName << "\" \"color tex2\" [" << curGeometry.diffuse.x << ' ' << curGeometry.diffuse.y << ' ' << curGeometry.diffuse.z << "]" << endl;
                file << "Material \"matte\" \"texture Kd\" \"" << localTextureName << "\"" << endl;
            }

            file << "AttributeBegin" << endl;
            file << "Shape \"trianglemesh\"" << endl;
            file << "  \"point P\" [" << endl;
            for (const ml::TriMeshf::Vertexf& vertex : vertices)
            {
                const vec3f pos = modelToWorld.transformAffine(vertex.position);
                file << "    " << pos.x << ' ' << pos.y << ' ' << pos.z << endl;
            }
            file << "  ]" << endl;

            if (meshData.texture != NULL)
            {
                file << "  \"float st\" [" << endl;
                for (const ml::TriMeshf::Vertexf& vertex : vertices)
                {
                    file << "    " << vertex.texCoord.x << ' ' << vertex.texCoord.y << endl;
                }
                file << "  ]" << endl;
            }

            file << "  \" integer indices\" [" << endl;
            for (UINT indexIndex = 0; indexIndex < frontFacingIndices.size(); indexIndex += 3)
            {
                file << "   " << frontFacingIndices[indexIndex + 0] << ' ' << frontFacingIndices[indexIndex + 1] << ' ' << frontFacingIndices[indexIndex + 2] << endl;
            }
            file << "  ]" << endl;
            file << "AttributeEnd" << endl;
        }
    }
}

void AssetRenderer::renderPOVRayObjects(AssetManager &assets, ml::GraphicsDevice &g, const string& filename, const Cameraf &camera, const Scene &s, const vector<vec3f> &objectColors)
{
    boost::filesystem::path path(filename);
    std::string path_models = path.parent_path().string() + "/models";
    if (!boost::filesystem::exists(path_models))
        boost::filesystem::create_directory(path_models);
    std::ofstream file(filename, std::ofstream::out | std::ofstream::app);
    for (UINT objectIndex = 0; objectIndex < s.objects.size(); objectIndex++)
    {
        // Objects with negative colors are a signal they should be skipped
        if (objectColors[objectIndex].x >= 0.0f)
        {
            const std::string filename_object = s.objects[objectIndex].object.modelId + "_" + s.id + "_" + to_string(objectIndex) + ".inc";
            file << "#include \"models/" << filename_object << "\"" << endl;
            renderPOVRayModel(assets, g, path_models + "/" + filename_object, camera, s.objects[objectIndex].object.modelId, s.objects[objectIndex].modelToWorld, objectColors[objectIndex]);
        }
    }
    file.close();

    return;
}

void AssetRenderer::renderPOVRayModel(AssetManager &assets, ml::GraphicsDevice &g, const string& filename, const Cameraf &camera, const string &modelId, const mat4f &modelToWorld, const vec3f &color)
{
    boost::filesystem::path path(filename);
    std::string directory = path.parent_path().parent_path().string() + "/";
    std::string directory_texturesPNG = directory + "/texturesPNG/";
    if (!boost::filesystem::exists(directory_texturesPNG))
        boost::filesystem::create_directory(directory_texturesPNG);
    std::string filename_stem = path.stem().string();

    std::ofstream file(filename);

    const mat4f modelToCamera = camera.getCamera() * modelToWorld; //cameraPerspective.getRotation()*ml::point3d<float>(0.0f, 0.0f, -1.0f);

    const ModelAssetData &ModelAssetData = assets.loadModel(g, modelId);
    int empty_texture_name_count = 0;
    for (const ModelMeshData &meshData : ModelAssetData.data)
    {
        std::string localTextureName, localTextureFilename;
        localTextureName = filename_stem;
        for (size_t i = 0; i < localTextureName.size(); ++i) {
            if (localTextureName[i] == '.' || localTextureName[i] == '-') {
                localTextureName[i] = '_';
            }
        }
        if (meshData.texture != NULL)
        {
            std::string textureName = boost::filesystem::path(meshData.texture->name).stem().string();
            if (textureName.empty()) {
                textureName = ml::util::zeroPad(empty_texture_name_count++, 4);
            }
            const std::string textureFilenamePNG = directory_texturesPNG + "/" + textureName + ".png";

            localTextureName = localTextureName + "_" + textureName;
            localTextureFilename = textureName;

            if (!ml::util::fileExists(textureFilenamePNG))
            {
				Bitmap bmp = meshData.texture->texture.getImage();
                size_t width = bmp.getWidth(), height = bmp.getHeight();

                std::vector<ml::vec3f> rawData(width * height);
                for (UINT y = 0; y < height; y++)
                {
                    for (UINT x = 0; x < width; x++)
                    {
                        ml::vec3f v = ml::vec3f(RGBColor(bmp(x, y)));
                        rawData[y * width + x] = v;
                    }
                }

                ml::BaseImage<vec3f> baseImage((UINT)width, (UINT)height, &(rawData[0]));
                ml::FreeImageWrapper::saveImage(textureFilenamePNG, baseImage);
            }
        }

        std::vector<UINT> frontFacingIndices;
        const auto &vertices = meshData.meshOriginal.getTriMesh().getVertices();
        for (const vec3ui& index : meshData.meshOriginal.getTriMesh().getIndices())
        {
            const UINT i0 = index[0];
            const UINT i1 = index[1];
            const UINT i2 = index[2];

            const vec3f v0 = modelToCamera.transformAffine(vertices[i0].position);
            const vec3f v1 = modelToCamera.transformAffine(vertices[i1].position);
            const vec3f v2 = modelToCamera.transformAffine(vertices[i2].position);
            vec3f normal = ml::math::triangleNormal(v0, v1, v2);

            bool frontFacing = normal.z >= 0.0f;

            if (frontFacing)
            {
                frontFacingIndices.push_back(i0);
                frontFacingIndices.push_back(i1);
                frontFacingIndices.push_back(i2);
            }
        }

        bool tintedColor = (vec3f::distSq(color, vec3f(1.0f, 1.0f, 1.0f)) > 0.0f);

        if (frontFacingIndices.size() > 0)
        {
            //if (meshData.material.m_shiny > 0)
            //{
            //file << "Material \"glass\"" << endl;
            //file << "  \"color Kt\" [" << curGeometry.diffuse.x << ' ' << curGeometry.diffuse.y << ' ' << curGeometry.diffuse.z << "]" << endl;
            //file << "  \"color Kr\" [" << curGeometry.diffuse.x << ' ' << curGeometry.diffuse.y << ' ' << curGeometry.diffuse.z << "]" << endl;
            //}
            //else
            if (meshData.texture == NULL || tintedColor)
            {
                ml::vec3f diffuse = meshData.material.m_diffuse.getVec3();
                if (tintedColor)
                    diffuse = ml::math::max(diffuse, vec3f(0.25f, 0.25f, 0.25f));

                file << "#declare " << localTextureName << " = texture{ pigment{ rgb <"
                    << diffuse.x * color.x << ' ' << diffuse.y * color.y << ' ' << diffuse.z * color.z
                    << "> filter 0 } finish{ ambient ambient_value diffuse diffuse_value phong phong_value } };";
            }
            else
            {
                file << "#declare " << localTextureName << " = texture{ pigment{ image_map {png \"texturesPNG/"
                    << localTextureFilename
                    << "\"}} finish{ ambient ambient_value diffuse diffuse_value phong phong_value } };\n";
            }

            file << "mesh {\n";
            for (UINT indexIndex = 0; indexIndex < frontFacingIndices.size(); indexIndex += 3)
            {
                int idx_0 = frontFacingIndices[indexIndex + 0];
                int idx_1 = frontFacingIndices[indexIndex + 1];
                int idx_2 = frontFacingIndices[indexIndex + 2];

                const ml::TriMeshf::Vertexf& v_0 = vertices[idx_0];
                const ml::TriMeshf::Vertexf& v_1 = vertices[idx_1];
                const ml::TriMeshf::Vertexf& v_2 = vertices[idx_2];

                const vec3f p_0 = modelToWorld.transformAffine(v_0.position);
                const vec3f p_1 = modelToWorld.transformAffine(v_1.position);
                const vec3f p_2 = modelToWorld.transformAffine(v_2.position);

                file << "  triangle {\n";
                file << "    <" << p_0.x << "," << p_0.y << "," << p_0.z << ">, "
                    << "<" << p_1.x << ", " << p_1.y << ", " << p_1.z << ">, "
                    << "<" << p_2.x << ", " << p_2.y << ", " << p_2.z << ">\n";

                if (meshData.texture != NULL)
                {
                    const vec2f t_0 = v_0.texCoord;
                    const vec2f t_1 = v_1.texCoord;
                    const vec2f t_2 = v_2.texCoord;
                    file << "    uv_vectors <" << t_0.x << ", " << t_0.y << ">, "
                        << "<" << t_1.x << ", " << t_1.y << ">, "
                        << "<" << t_2.x << ", " << t_2.y << ">\n";
                }
                file << "  }\n";
            }
            file << "  texture {" << localTextureName << "}\n";
            file << "}\n\n";
        }
    }

    return;
}

void AssetRenderer::renderMesh(ml::GraphicsDevice &g, const mat4f &cameraPerspective, const ml::D3D11TriMesh &mesh, const vec3f &color)
{
    renderMesh(g, cameraPerspective, mat4f::identity(), mesh, color);
}

void AssetRenderer::renderMesh(ml::GraphicsDevice &g, const mat4f &cameraPerspective, const mat4f &meshToWorld, const ml::D3D11TriMesh &mesh, const vec3f &color)
{
    m_shaders.bindShaders("basicColor");

    AssetRendererConstantBuffer constants;

    constants.worldViewProj = cameraPerspective * meshToWorld;
    constants.modelColor = ml::vec4f(color, 1.0f);
    m_constants.updateAndBind(constants, 0);
    mesh.render();
}

void AssetRenderer::renderSphere(ml::GraphicsDevice &g, const mat4f &cameraPerspective, const vec3f &center, float radius, const vec3f &color)
{
    renderMesh(g, cameraPerspective, mat4f::translation(center) * mat4f::scale(-radius), m_sphere, color);
}

void AssetRenderer::renderCylinder(ml::GraphicsDevice &g, const mat4f &cameraPerspective, const vec3f &p0, const vec3f &p1, const vec3f &color)
{
    renderMesh(g, cameraPerspective, mat4f::translation(p0) * mat4f::face(vec3f::eZ, p1 - p0) * mat4f::scale(1.0f, 1.0f, vec3f::dist(p0, p1)), m_cylinder, color);
}

void AssetRenderer::renderBox(ml::GraphicsDevice &g, const mat4f &cameraPerspective, const vec3f &center, float radius, const vec3f &color)
{
    renderMesh(g, cameraPerspective, mat4f::translation(center) * mat4f::scale(radius), m_box, color);
}

void AssetRenderer::renderPolygon(ml::GraphicsDevice &g, const mat4f &cameraPerspective, const Polygonf &poly, const vec3f &color, float height)
{
    for (const auto &segment : poly.segments())
    {
        renderCylinder(g, cameraPerspective, vec3f(segment.p0(), height), vec3f(segment.p1(), height), color);
        renderSphere(g, cameraPerspective, vec3f(segment.p0(), height), 0.02f, color);
    }
}
