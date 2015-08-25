
#include "main.h"

void ScoreMode::init(ml::ApplicationData& _app, AppState &_state)
{
    app = &_app;
    state = &_state;

    selectedObjectIndex = -1;
}

void ScoreMode::newScene()
{
    const Scene &scene = state->database.scenes.getScene(state->curScene);
    selectedObjectIndex = -1;

    //
    // Create accelerators
    //
    objectAccelerators.clear();
    objectAccelerators.resize(scene.objects.size());

    for (UINT objectIndex = 0; objectIndex < scene.objects.size(); objectIndex++)
    {
        const ObjectInstance &object = scene.objects[objectIndex];
        const ModelAssetData &ModelAssetData = state->assets.loadModel(app->graphics.castD3D11(), object.object.modelId);

        vector< pair<const ml::TriMeshf *, mat4f> > meshes;
        for (const auto &mesh : ModelAssetData.data)
			meshes.push_back(std::make_pair(&mesh.meshOriginal.getTriMesh(), object.modelToWorld));

        objectAccelerators[objectIndex].build(meshes);
    }

    //
    // Create score visualizations
    //
    objectVizData.clear();
    objectVizData.resize(scene.objects.size());

    for (UINT objectIndex = 0; objectIndex < scene.objects.size(); objectIndex++)
    {
        const ObjectInstance &object = scene.objects[objectIndex];
        vector<ml::TriMeshf> meshes;

        //
        // TODO: handle multiply-owned objects
        //
        int ownerIndex = -1;
        
        auto agents = scene.agentsFromObjectIndex(objectIndex);
        if (agents.size() > 0)
        ownerIndex = agents[0];

        if (ownerIndex != -1 && state->database.interactionMaps.maps.count(object.object.modelId) > 0)
        {
            const InteractionMap &iMap = state->database.interactionMaps.maps[object.object.modelId];
            const Agent &agent = scene.agents[ownerIndex];
            for (const auto &entry : iMap.entries)
            {
                for (const auto &centroid : entry.second.centroids)
                {
                    vec3f color = vec3f(entry.second.interactionColor());

                    vec3f centroidWorldPos = object.modelToWorld * centroid.pos;
                    ml::Rayf ray(agent.headPos, centroidWorldPos - agent.headPos);
                    
                    ml::TriMeshRayAcceleratorf::Intersection intersection;
                    UINT intersectObjectIndex;
                    if (ml::TriMeshRayAcceleratorf::getFirstIntersection(ray, ml::util::toVecConstPtr(objectAccelerators), intersection, intersectObjectIndex))
                    {
                        double centroidDist = ml::dist(agent.headPos, centroidWorldPos);
                        double intersectDist = ml::dist(agent.headPos, intersection.getSurfacePosition());

                        if (intersectDist > centroidDist + 0.01)
                        {
                            cout << "A ray intersected behind the centroid" << endl;
                        }

                        bool occluded = (fabs(centroidDist - intersectDist) > 0.01);

                        if (occluded)
                            color *= 0.4f;
                    }
                    else
                    {
                        cout << "A ray did not intersect the scene" << endl;
                    }

                    meshes.push_back(ml::Shapesf::sphere(0.01f, centroidWorldPos, 5, 5, ml::vec4f(color, 1.0f)));
                    meshes.push_back(ml::Shapesf::cylinder(centroidWorldPos, agent.headPos, 0.003f, 5, 7, ml::vec4f(color, 1.0f)));
                }
            }
        }

        objectVizData[objectIndex].unifiedMesh.load(app->graphics, ml::meshutil::createUnifiedMesh(meshes));
    }
}

void ScoreMode::render()
{
    const Scene &scene = state->database.scenes.getScene(state->curScene);

    vector<vec3f> objectColors(scene.objects.size(), vec3f(1, 1, 1));

    for (UINT objectIndex = 0; objectIndex < scene.objects.size(); objectIndex++)
    {
        auto mode = objectVizData[objectIndex].mode;
        if (mode == ObjectScoreVizData::Normal && objectIndex == selectedObjectIndex)
            objectColors[objectIndex] = vec3f(1.5f, 1.5f, 1.5f);
        if (mode == ObjectScoreVizData::Green)
            objectColors[objectIndex] = vec3f(0.5f, 1.0f, 0.5f);
        if (mode == ObjectScoreVizData::Red)
            objectColors[objectIndex] = vec3f(1.0f, 0.5f, 0.5f);
        if (mode == ObjectScoreVizData::Hidden)
            objectColors[objectIndex] = vec3f(2.0f, 2.0f, 2.0f);
    }

    state->renderer.renderObjects(state->assets, app->graphics.castD3D11(), state->camera.getCameraPerspective(), scene, objectColors, false);

    for (const Agent &a : scene.agents)
    {
        vec3f color = vec3f(0.8f, 0.8f, 0.8f);
        state->renderer.renderAgent(app->graphics.castD3D11(), state->camera.getCameraPerspective(), a, color);
    }

    for (UINT objectIndex = 0; objectIndex < scene.objects.size(); objectIndex++)
    {
        if (selectedObjectIndex == -1 || selectedObjectIndex == objectIndex)
        {
            //state->renderer.renderMesh(app->graphics.castD3D11(), state->camera.cameraPerspective(), objectVizData[objectIndex].unifiedMesh, vec3f(1.0f, 1.0f, 1.0f));
        }
    }
}

void ScoreMode::renderPBRT(const string &path)
{
    PBRT::CopyPBRTFiles();
    PBRT::PBRTPreamble(state->camera, synthParams().PBRTResolution);

    const Scene &scene = state->database.scenes.getScene(state->curScene);

    vector<vec3f> objectColors(scene.objects.size(), vec3f(1, 1, 1));

    for (UINT objectIndex = 0; objectIndex < scene.objects.size(); objectIndex++)
    {
        auto mode = objectVizData[objectIndex].mode;
        if (mode == ObjectScoreVizData::Green)
            objectColors[objectIndex] = vec3f(0.5f, 1.0f, 0.5f);
        if (mode == ObjectScoreVizData::Red)
            objectColors[objectIndex] = vec3f(1.0f, 0.5f, 0.5f);
        if (mode == ObjectScoreVizData::Hidden)
            objectColors[objectIndex] = vec3f(-1.0f, -1.0f, -1.0f);
    }

    state->renderer.renderPBRTObjects(state->assets, app->graphics, PBRT::PBRTFilename(), state->camera, scene, objectColors);

    PBRT::PBRTFinalize();

    // renderPOVRay here
    POVRay::POVRayPreamble(state->camera, 800);
    state->renderer.renderPOVRayObjects(state->assets, app->graphics, POVRay::POVRayMainFile(), state->camera, scene, objectColors);
    POVRay::POVRayFinalize();
}

void ScoreMode::keyDown(UINT key)
{
    Scene &scene = state->database.scenes.getScene(state->curScene);

    if (key == KEY_ESCAPE) selectedObjectIndex = -1;

    int modeDelta = 0;
    if (key == KEY_Y) modeDelta = -1;
    if (key == KEY_U) modeDelta = 1;
    if (modeDelta && selectedObjectIndex != 0)
        objectVizData[selectedObjectIndex].mode = (ObjectScoreVizData::VizMode)ml::math::mod(objectVizData[selectedObjectIndex].mode + modeDelta, ObjectScoreVizData::ModeCount);

    if (key == KEY_O)
    {
        const string hashString = ml::util::encodeBytes(ml::util::hash32(ml::util::randomUniform()));
        renderSceneD3D(synthParams().synthesisDir + "figures/" + "figMode_" + hashString + ".png");
    }
}

void ScoreMode::mouseDown(ml::MouseButtonType button)
{
    bool shift = (GetAsyncKeyState(VK_SHIFT) != 0);
    Scene &scene = state->database.scenes.getScene(state->curScene);

    float screenX = (float)app->input.mouse.pos.x / (app->window.getWidth() - 1.0f);
    float screenY = (float)app->input.mouse.pos.y / (app->window.getHeight() - 1.0f);
    ml::Rayf ray = state->camera.getScreenRay(screenX, screenY);

    if (shift && button == ml::MouseButtonLeft)
    {
        selectedObjectIndex = -1;

        ml::TriMeshRayAcceleratorf::Intersection intersection;
        UINT objectIndex;
        if (ml::TriMeshRayAcceleratorf::getFirstIntersection(ray, ml::util::toVecConstPtr(objectAccelerators), intersection, objectIndex))
        {
            selectedObjectIndex = objectIndex;
        }
    }
}

string ScoreMode::helpText() const
{
    string text;
    text += "Shift + left click: select object|";
    text += "YU: change object mode|";
    text += "O: output scene|";
    return text;
}

vector< pair<string, RGBColor> > ScoreMode::statusText() const
{
    const Scene &scene = state->database.scenes.getScene(state->curScene);

    vector< pair<string, RGBColor> > result;

    if (selectedObjectIndex == -1)
        result.push_back(std::make_pair(string("No object selected"), RGBColor::White));
    else
    {
        const ObjectInstance &o = scene.objects[selectedObjectIndex];
        result.push_back(std::make_pair("Object viz mode: " + to_string(objectVizData[selectedObjectIndex].mode), RGBColor::White));
        //result.push_back(std::make_pair("Dist to wall: " + to_string(o.object.distanceToWall), RGBColor::White));
        //result.push_back(std::make_pair("Frame rotation: " + to_string(o.object.coordianteFrameRotation), RGBColor::White));
    }

    return result;
}

void ScoreMode::renderSceneD3D(const string &outputFilename) const
{
    cout << "Rendering to " << outputFilename << endl;
    const Scene &scene = state->database.scenes.getScene(state->curScene);

    ml::Cameraf camera = state->camera;
    camera.updateAspectRatio(1.0f);

    ml::D3D11RenderTarget renderTarget;
    renderTarget.load(app->graphics, 1024, 1024);
    renderTarget.clear(vec4f(0.0f, 0.0f, 0.0f, 1.0f));
    renderTarget.bind();

    for (UINT objectIndex = 0; objectIndex < scene.objects.size(); objectIndex++)
    {
        vec3f color(1.0f, 1.0f, 1.0f);
        auto mode = objectVizData[objectIndex].mode;
        if (mode == ObjectScoreVizData::Green)
            color = vec3f(0.5f, 1.0f, 0.5f);
        if (mode == ObjectScoreVizData::Red)
            color = vec3f(1.0f, 0.5f, 0.5f);

        if (mode != ObjectScoreVizData::Hidden)
            state->renderer.renderObject(state->assets, app->graphics.castD3D11(), camera.getCameraPerspective(), scene.objects[objectIndex], color, false);
    }

	Bitmap bmp;
    renderTarget.captureColorBuffer(bmp);
    ml::LodePNG::save(bmp, outputFilename);

    app->graphics.castD3D11().bindRenderTarget();
}
