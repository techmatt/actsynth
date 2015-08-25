
#include "main.h"

void InteractionMapMode::init(ml::ApplicationData& _app, AppState &_state)
{
    app = &_app;
    state = &_state;

    std::set<string> modelList;
    for (auto &s : state->database.scenes.scenes)
        for (auto &m : s.second.objects)
            if (!ml::util::startsWith(m.object.modelId, "wss.room"))
                modelList.insert(m.object.modelId);
    auto lines = ml::util::getFileLines(synthParams().synthesisDir + "additionalModels.txt", 3);

    //
    //
    //
    //lines.clear();

    for (const string& l : lines) {
      modelList.insert(l);
      auto& m = state->database.models.getModel(l);
      if (state->database.categories.categories.count(m.categoryName) == 0) {
        auto& catDb = const_cast<CategoryDatabase&>(state->database.categories);
        catDb.categories[m.categoryName].name = m.categoryName;
        cout << "Adding category: " << m.categoryName << endl;
      }
    }
    models = vector<string>(modelList.begin(), modelList.end());

    std::sort(models.begin(), models.end(), [&](const string &a, const string &b)
    {
        return state->database.models.getModel(a).categoryName < state->database.models.getModel(b).categoryName;
    });

    modelIndex = 0;
    model = nullptr;

    interactionTypes = state->database.interactionMaps.allInteractionMapTypes();
    interactionIndex = 0;

    camera = ml::Camera<float>("-0.569868 -1.10533 0.554568 0.906308 -0.422618 0 0.384483 0.824526 -0.414723 0.186042 0.370667 0.90994 0 0 1 60 1.39502 0.01 1000");
    camera.updateAspectRatio((float)app->window.getWidth() / app->window.getHeight());
}

void InteractionMapMode::loadModel(const string &modelId)
{
    int index = ml::util::indexOf(models, modelId);
    if (index == -1)
    {
        cout << "Model not found: " << modelId << endl;
        return;
    }
    modelIndex = index;
    model = nullptr;
}

void InteractionMapMode::render()
{
    if (model == nullptr)
    {
        const string modelId = models[modelIndex];
        if (state->database.models.models.count(modelId) == 0)
        {
            cout << "Model not found: " << modelId << endl;
            return;
        }

        model = &state->database.models.models[modelId];

        const ModelAssetData &ModelAssetData = state->assets.loadModel(app->graphics.castD3D11(), modelId);

        ml::bbox3f box;
        for (const auto &mesh : ModelAssetData.data)
            box.include(mesh.meshOriginal.boundingBox());

        modelToWorld = mat4f::scale(1.0f / box.getExtent().length()) * mat4f::translation(-box.getCenter());

        vector< pair<const ml::TriMeshf *, mat4f> > meshes;
        for (const auto &mesh : ModelAssetData.data)
			meshes.push_back(std::make_pair(&mesh.meshOriginal.getTriMesh(), modelToWorld));

        accelerator.build(meshes);
        selectedCentroidIndex = -1;
    }

    state->renderer.renderModel(state->assets, app->graphics.castD3D11(), camera.getCameraPerspective(), model->id, modelToWorld, vec3f(1.0f, 1.0f, 1.0f), false);

    for (const string &interactionType : interactionTypes)
    {
        if (state->database.interactionMaps.hasInteractionMapEntry(models[modelIndex], interactionType))
        {
            const InteractionMapEntry &entry = state->database.interactionMaps.getInteractionMapEntry(models[modelIndex], interactionType);
            vec3f interactionColor = InteractionMapEntry::interactionColor(interactionType);

            int centroidIndex = 0;
            for (const auto &centroid : entry.centroids)
            {
                vec3f color = interactionColor;
                if (centroidIndex == selectedCentroidIndex)
                    color += vec3f(0.3f, 0.3f, 0.3f);
                state->renderer.renderSphere(app->graphics.castD3D11(), camera.getCameraPerspective(), modelToWorld * centroid.pos, 0.025f, color);
                centroidIndex++;
            }
        }
    }
}

void InteractionMapMode::keyDown(UINT key)
{
    bool ctrl = (GetAsyncKeyState(VK_CONTROL) != 0);

    if (key == KEY_ESCAPE) selectedCentroidIndex = -1;
    if (key == KEY_R)
    {
        if (state->database.interactionMaps.hasInteractionMapEntry(models[modelIndex], interactionTypes[interactionIndex]))
            state->database.interactionMaps.createInteractionMapEntry(models[modelIndex], interactionTypes[interactionIndex]).centroids.clear();
        dirty = true;
    }
    if (selectedCentroidIndex != -1 && (key == KEY_BACKSPACE || key == KEY_DELETE))
    {
        auto &centroids = state->database.interactionMaps.createInteractionMapEntry(models[modelIndex], interactionTypes[interactionIndex]).centroids;
        std::swap(centroids[selectedCentroidIndex], centroids.back());
        centroids.pop_back();
        selectedCentroidIndex = -1;
        dirty = true;
    }
    if (key == KEY_C)
    {
        sampleCylinderPoints();
    }

    int modelDelta = 0, interactionDelta = 0;
    if (key == KEY_UP) interactionDelta = -1;
    if (key == KEY_DOWN) interactionDelta = 1;
    if (key == KEY_LEFT) modelDelta = -1;
    if (key == KEY_RIGHT) modelDelta = 1;

    if (modelDelta != 0)
    {
        if (dirty && model != nullptr && state->database.interactionMaps.maps.count(model->id) > 0)
        {
            cout << "Saving interaction map for " << model->id << endl;
            state->database.interactionMaps.maps[model->id].save(R"(V:\data\synthesis\interactionMaps\)" + model->id + ".arv");
            dirty = false;
        }
        
        if (ctrl)
        {
            for (modelIndex = 0; modelIndex < models.size(); modelIndex++)
            {
                const string &categoryName = state->database.models.getModel(models[modelIndex]).categoryName;
                if (state->database.interactionMaps.maps.count(models[modelIndex]) == 0 && categoryName != "Table" && categoryName != "SideTable")
                {
                    break;
                }
            }
            modelIndex = ml::math::mod(modelIndex, models.size());
        }
        else
            modelIndex = ml::math::mod(modelIndex + modelDelta, models.size());

        model = nullptr;
    }
    if (interactionDelta != 0)
    {
        interactionIndex = ml::math::mod(interactionIndex + interactionDelta, interactionTypes.size());
    }
}

void InteractionMapMode::keyPressed(UINT key)
{
    float distance = 0.2f;
    float theta = 2.5f;

    if (key == KEY_W) { camera.move(distance); }
    if (key == KEY_S) { camera.move(-distance); }
    if (key == KEY_A) { camera.strafe(-distance); }
    if (key == KEY_D) { camera.strafe(distance); }
    if (key == KEY_E) { camera.jump(distance); }
    if (key == KEY_Q) { camera.jump(-distance); }
}

void InteractionMapMode::mouseWheel(int wheelDelta)
{
    const float distance = 0.002f;
    camera.move(distance * wheelDelta);
}

void InteractionMapMode::mouseMove()
{
    bool shift = (GetAsyncKeyState(VK_SHIFT) != 0);
    bool ctrl = (GetAsyncKeyState(VK_CONTROL) != 0);
    if (shift || ctrl) return;

    const float distance = 0.01f;
    const float theta = 0.25f;

    ml::vec2i posDelta = app->input.mouse.pos - app->input.prevMouse.pos;

    if (app->input.mouse.buttons[ml::MouseButtonRight])
    {
        camera.strafe(distance * posDelta.x);
        camera.jump(-distance * posDelta.y);
    }

    if (app->input.mouse.buttons[ml::MouseButtonLeft])
    {
        camera.lookRight(-theta * posDelta.x);
        camera.lookUp(theta * posDelta.y);
    }
}

void InteractionMapMode::mouseDown(ml::MouseButtonType button)
{
    bool shift = (GetAsyncKeyState(VK_SHIFT) != 0);

    float screenX = (float)app->input.mouse.pos.x / (app->window.getWidth() - 1.0f);
    float screenY = (float)app->input.mouse.pos.y / (app->window.getHeight() - 1.0f);
    ml::Rayf ray = camera.getScreenRay(screenX, screenY);

    if (shift && button == ml::MouseButtonLeft)
    {
        selectedCentroidIndex = -1;
        if (state->database.interactionMaps.hasInteractionMapEntry(models[modelIndex], interactionTypes[interactionIndex]))
        {
            const InteractionMapEntry &entry = state->database.interactionMaps.getInteractionMapEntry(models[modelIndex], interactionTypes[interactionIndex]);
            int centroidIndex = 0;
            for (const auto &centroid : entry.centroids)
            {
                ml::TriMeshAcceleratorBruteForcef accelerator;
                accelerator.build(state->renderer.sphereMesh().getTriMesh(), mat4f::translation(modelToWorld * centroid.pos) * mat4f::scale(0.025f));

                ml::TriMeshRayAccelerator<float>::Intersection intersection;
                if (accelerator.intersect(ray, intersection))
                    selectedCentroidIndex = centroidIndex;
                centroidIndex++;
            }
        }
    }
    if (shift && button == ml::MouseButtonRight)
    {
        ml::TriMeshRayAccelerator<float>::Intersection intersection;
        if (accelerator.intersect(ray, intersection))
        {
            InteractionMapEntry &entry = state->database.interactionMaps.createInteractionMapEntry(models[modelIndex], interactionTypes[interactionIndex]);
            if (selectedCentroidIndex == -1)
            {
                //
                // Create a new centroid
                //
                InteractionMapEntry::Centroid centroid;
                centroid.pos = modelToWorld.getInverse() * intersection.getSurfacePosition();
                centroid.meshIndex = intersection.getMeshIndex();
                centroid.triangleIndex = intersection.getTriangleIndex();
                centroid.uv = ml::vec2f(intersection.u, intersection.v);

                entry.centroids.push_back(centroid);
            }
            else
            {
                //
                // Move the current centroid
                //
                InteractionMapEntry::Centroid &centroid = entry.centroids[selectedCentroidIndex];
                centroid.pos = modelToWorld.getInverse() * intersection.getSurfacePosition();
				centroid.meshIndex = intersection.getMeshIndex();
				centroid.triangleIndex = intersection.getTriangleIndex();
				centroid.uv = ml::vec2f(intersection.u, intersection.v);
            }
            dirty = true;
        }
    }
}

void InteractionMapMode::sampleCylinderPoints()
{
    auto randomRay = []()
    {
        const float theta = ml::RNG::global.uniform(0.0f, 2.0f * ml::math::PIf);
        const float radius = 10.0f;
        const float height = ml::RNG::global.uniform(-1.0f, 1.0f);
        vec3f pos = vec3f(radius * cosf(theta), radius * sinf(theta), height);
        return ml::Rayf(pos, vec3f(-pos.x, -pos.y, 0.0f));
    };

    vector<vec3f> hitPoints;
    vector< ml::TriMeshAcceleratorBruteForcef::Intersection > hitIntersections;

    const UINT rayCount = 1000;
    for (int rayIndex = 0; rayIndex < rayCount; rayIndex++)
    {
        ml::Rayf ray = randomRay();

        ml::TriMeshRayAccelerator<float>::Intersection intersection;
        accelerator.intersect(ray, intersection);
        if (intersection.isValid())
        {
            hitPoints.push_back(intersection.getSurfacePosition());
            hitIntersections.push_back(intersection);
        }
    }

    if (hitPoints.size() == 0)
    {
        cout << "No points found" << endl;
        return;
    }

    ml::KMeansClustering<vec3f, ml::vec3fKMeansMetric> clustering;
    clustering.cluster(hitPoints, 20, 0, false);

    InteractionMapEntry &entry = state->database.interactionMaps.createInteractionMapEntry(models[modelIndex], interactionTypes[interactionIndex]);
    entry.centroids.clear();

    for (UINT clusterIndex = 0; clusterIndex < clustering.clusterCount(); clusterIndex++)
    {
        vec3f clusterCenter = clustering.clusterCenter(clusterIndex);

        size_t minIndex = ml::util::minIndex( hitPoints, [&](const vec3f &v) { return vec3f::distSq(v, clusterCenter); });
        auto intersection = hitIntersections[minIndex];
        
        InteractionMapEntry::Centroid centroid;
        centroid.pos = modelToWorld.getInverse() * intersection.getSurfacePosition();
        centroid.meshIndex = intersection.getMeshIndex();
        centroid.triangleIndex = intersection.getTriangleIndex();
		centroid.uv = ml::vec2f(intersection.u, intersection.v);

        entry.centroids.push_back(centroid);
    }
    dirty = true;
}

vector< pair<string, RGBColor> > InteractionMapMode::statusText() const
{
    const string &interaction = interactionTypes[interactionIndex];
    const Model &model = state->database.models.getModel(models[modelIndex]);
    
    vector< pair<string, RGBColor> > result;
    result.push_back(std::make_pair("Category: " + model.categoryName, RGBColor::White));
    result.push_back(std::make_pair("Interaction type: " + interaction, InteractionMapEntry::interactionColor(interaction)));
    result.push_back(std::make_pair("Model: " + model.id, InteractionMapEntry::interactionColor(interaction)));
    return result;
}

string InteractionMapMode::helpText() const
{
    string text;
    text += "Arrow keys: change model and interaction type|";
    text += "Ctrl+left/right: move to next unannotated model|";
    text += "Shift + left click: select centroid|";
    text += "Shift + right click: move/create centroid|";
    text += "R: reset centroids|";
    text += "C: sample cylinder points";
    return text;
}

void InteractionMapMode::recategorizeModel(const string &newName) const
{
    ofstream file;
    file.open(synthParams().synthesisDir + "categoryOverride.tsv", std::fstream::out | std::fstream::app);

    file << model->id << '\t' << newName << endl;
}
