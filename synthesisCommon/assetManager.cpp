
#include "main.h"

void AssetManager::loadAllModels(ml::GraphicsDevice &g, const set<string> &categories)
{
	cout << "Loading all assets..." << endl;
	for (const auto &category : database->categories.categories)
	{
		if (categories.count(category.first) > 0)
		{
			cout << "Loading " << category.first << " (" << category.second.instances.size() << " instances)..." << endl;
			for (const auto &instance : category.second.instances)
			{
				const auto &data = loadModel(g, instance.model->id);
				data.makeAllCaches();
			}
		}
	}
	cout << "Done loading assets" << endl;
}

void AssetManager::clear()
{
    textures.clear();
    models.clear();
}

void AssetManager::clearCategory(const string &category)
{
    cout << "Clearing all " << category << " models..." << endl;
    vector<string> modelIds;
    for (const auto &m : models)
        if (database->models.getModel(m.first).categoryName == category)
            modelIds.push_back(m.first);

    for (const string &s : modelIds)
        models.erase(s);
}

const ModelAssetData& AssetManager::loadModel(ml::GraphicsDevice &g, const string &modelId)
{
	if (models.find(modelId) == models.end())
	{
		ModelAssetData &renderData = models[modelId];

		vector<ml::TriMeshf> meshes, meshesSimplified;
		vector<ml::Materialf> materials;
		util::loadCachedModel(modelId, meshes, meshesSimplified, materials);

		//
		// some model categories, such as wine glasses, do not simplify well and the original mesh should be used instead.
		//
		if (database->categories.getCategory(database->models.getModel(modelId).categoryName).isNoSimplify)
		{
			meshesSimplified = meshes;
		}

		if (meshes.size() == 0 || meshes.size() != materials.size())
		{
			cout << "util::loadCachedModel failed for " << modelId << endl;
			renderData.data.push_back(ModelMeshData());
			ModelMeshData &meshData = renderData.data.back();
			meshData.meshOriginal.load(g, ml::Shapesf::box());
			meshData.meshSimplified = meshData.meshOriginal;
		}
		else
		{
			renderData.data.resize(meshes.size());
			for (UINT meshIndex = 0; meshIndex < meshes.size(); meshIndex++)
			{
				ModelMeshData &meshData = renderData.data[meshIndex];
				meshData.meshOriginal.load(g, std::move(meshes[meshIndex]));
				meshData.meshSimplified.load(g, std::move(meshesSimplified[meshIndex]));
                meshData.material = std::move(materials[meshIndex]);
                if (meshData.material.m_TextureFilename_Kd.size() >= 4)
                {
                    meshData.texture = &loadTexture(g, modelId, meshData.material.m_TextureFilename_Kd);
                    if (meshData.texture->texture.getImage().size() == 0)
                    {
                        cout << "Empty texture: " << meshData.material.m_TextureFilename_Kd << " for " << modelId << endl;
                        meshData.texture = NULL;
                    }
                }
            }
        }
    }
    return models[modelId];
}

const TextureData& AssetManager::loadTexture(ml::GraphicsDevice &g, const string &modelId, const string &textureId)
{
    string fullTextureId = textureId;
    if (ml::util::startsWith(modelId, "hao."))
    {
        fullTextureId = "hao-" + database->models.getModel(modelId).categoryName + "-" + ml::util::split(modelId, ".").back() + "-" + ml::util::remove(textureId, vector<string>({ "./models/untitled/", "./images/" }));
    }

    if (textures.find(fullTextureId) == textures.end())
	{
        TextureData &textureData = textures[fullTextureId];
        textureData.name = fullTextureId;

        Bitmap bmp(1, 1);

        if (!synthParams().skipTextureLoading)
            bmp = util::loadCachedTexture(fullTextureId);

        textureData.texture.load(g, bmp);
	}
    return textures[fullTextureId];
}

void AssetManager::swapTexture(ml::GraphicsDevice &g, const string &textureId, const string &filename)
{
    ml::D3D11Texture2D &tex = textures[textureId].texture;

    Bitmap image;
    ml::FreeImageWrapper::loadImage(filename, image);

    tex.load(g, image);
}