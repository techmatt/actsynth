
#include "main.h"

void AgentMode::init(ml::ApplicationData& _app, AppState &_state)
{
    app = &_app;
    state = &_state;

    activityList = Activity::activityList();
    activityColors = Activity::activityColors();

    agentColorList.push_back(vec3f(0.8f, 0.3f, 0.3f));
    agentColorList.push_back(vec3f(0.8f, 0.8f, 0.3f));
    agentColorList.push_back(vec3f(0.3f, 0.3f, 0.8f));
    agentColorList.push_back(vec3f(0.3f, 0.8f, 0.3f));
    agentColorList.push_back(vec3f(0.3f, 0.8f, 0.8f));
    agentColorList.push_back(vec3f(0.8f, 0.3f, 0.8f));
    agentColorList.push_back(vec3f(0.8f, 0.8f, 0.8f));

    dirty = false;
}

void AgentMode::closeScene()
{
    if (dirty)
    {
        const Scene &scene = state->database.scenes.getScene(state->curScene);
        cout << "Saving agents for " << scene.id << endl;

        string tag = "";

        scene.saveAgentAnnotations(R"(V:\data\synthesis\agentAnnotations)" + tag + "/" + scene.id + ".arv");
        dirty = false;
    }
}

void AgentMode::newScene()
{
    const Scene &scene = state->database.scenes.getScene(state->curScene);
    selectedAgentIndex = -1;

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
}

void AgentMode::render()
{
    const Scene &scene = state->database.scenes.getScene(state->curScene);

    if (state->mode == VizzerModeAgentPlacement)
    {
        vector<vec3f> objectColors(scene.objects.size(), vec3f(1, 1, 1));
        state->renderer.renderObjects(state->assets, app->graphics.castD3D11(), state->camera.getCameraPerspective(), scene, objectColors, false);

        int agentIndex = 0;
        for (const Agent &a : scene.agents)
        {
            vec3f color = activityColors[a.activity];
            if (agentIndex != selectedAgentIndex)
                color *= 0.6f;
            state->renderer.renderAgent(app->graphics.castD3D11(), state->camera.getCameraPerspective(), a, color);
            agentIndex++;
        }
    }

    if (state->mode == VizzerModeAgentOwnership)
    {
        if (agentColors.size() != scene.agents.size())
        {
            agentColors.clear();
            for (UINT i = 0; i < scene.agents.size(); i++)
                agentColors.push_back(agentColorList[i % agentColorList.size()]);
        }

        vec3f unboundColor = vec3f(0.5f, 0.5f, 0.5f);
        vector<vec3f> objectColors(scene.objects.size(), unboundColor);

        for (int agentIndex = 0; agentIndex < scene.agents.size(); agentIndex++)
        {
            for (UINT boundIndex : scene.agents[agentIndex].boundObjectIndices)
            {
                if (objectColors[boundIndex] != unboundColor)
                {
                    cout << "Object bound to multiple agents" << endl;
                }
                objectColors[boundIndex] = agentColors[agentIndex];
            }
        }

        state->renderer.renderObjects(state->assets, app->graphics.castD3D11(), state->camera.getCameraPerspective(), scene, objectColors, false);

        for (int agentIndex = 0; agentIndex < scene.agents.size(); agentIndex++)
        {
            vec3f color = agentColors[agentIndex];
            if (agentIndex != selectedAgentIndex)
                color *= 0.6f;
            state->renderer.renderAgent(app->graphics.castD3D11(), state->camera.getCameraPerspective(), scene.agents[agentIndex], color);
        }
    }
}

void AgentMode::renderPBRT(const string& path)
{
    PBRT::CopyPBRTFiles();
    PBRT::PBRTPreamble(state->camera, synthParams().PBRTResolution);

    const Scene &scene = state->database.scenes.getScene(state->curScene);
    vector<vec3f> objectColors(scene.objects.size(), vec3f(1, 1, 1));
    state->renderer.renderPBRTObjects(state->assets, app->graphics, PBRT::PBRTFilename(), state->camera, scene, objectColors);

    PBRT::PBRTFinalize();

    // renderPOVRay here
    POVRay::POVRayPreamble(state->camera, 800);
    state->renderer.renderPOVRayObjects(state->assets, app->graphics, POVRay::POVRayMainFile(), state->camera, scene, objectColors);
    POVRay::POVRayFinalize();
}

void AgentMode::assignProximityOwnership()
{
    Scene &scene = state->database.scenes.getScene(state->curScene);
    for (Agent &a : scene.agents)
        a.boundObjectIndices.clear();

    for (UINT objectIndex = 1; objectIndex < scene.objects.size(); objectIndex++)
    {
        bool supporting = false;
        for (Agent &a : scene.agents)
        {
            if (a.supportObjectIndex == objectIndex)
                supporting = true;
        }
        if (!supporting)
        {
            Agent *bestAgent = NULL;
            float bestAgentDistSq = std::numeric_limits<float>::max();
            for (Agent &a : scene.agents)
            {
                float distSq = (float)ml::distSq(scene.objects[objectIndex].worldBBox, a.headPos);
                if (distSq < bestAgentDistSq)
                {
                    bestAgentDistSq = distSq;
                    bestAgent = &a;
                }
            }
            bestAgent->boundObjectIndices.push_back(objectIndex);
        }
    }

    dirty = true;
}

void AgentMode::keyDown(UINT key)
{
    Scene &scene = state->database.scenes.getScene(state->curScene);

    if (key == KEY_ESCAPE) selectedAgentIndex = -1;
    if (key == KEY_R)
    {
        scene.agents.clear();
        selectedAgentIndex = -1;
        dirty = true;
    }
    if (key == KEY_P)
    {
        assignProximityOwnership();
    }

    if (selectedAgentIndex == -1)
        return;

    if (key == KEY_BACKSPACE || key == KEY_DELETE)
    {
        std::swap(scene.agents[selectedAgentIndex], scene.agents.back());
        scene.agents.pop_back();
        selectedAgentIndex = -1;
        dirty = true;
        return;
    }

    Agent &agent = scene.agents[selectedAgentIndex];

    if (state->mode == VizzerModeAgentPlacement)
    {
        int rotateDelta = 0, activityDelta = 0;
        if (key == KEY_LEFT) rotateDelta = 1;
        if (key == KEY_RIGHT) rotateDelta = -1;
        if (key == KEY_UP) activityDelta = -1;
        if (key == KEY_DOWN) activityDelta = 1;

        if (rotateDelta != 0)
        {
            mat4f rotation = mat4f::rotationZ(rotateDelta * 3.0f);
            agent.gazeDir = (rotation * agent.gazeDir).getNormalized();
            agent.rightDir = (rotation * agent.rightDir).getNormalized();
            dirty = true;
        }
        if (activityDelta != 0)
        {
            int activityIndex = ml::util::findFirstIndex(activityList, agent.activity);
            agent.activity = activityList[ml::math::mod(activityIndex + activityDelta, activityList.size())];
            dirty = true;
        }
    }
}

void AgentMode::mouseDown(ml::MouseButtonType button)
{
    bool shift = (GetAsyncKeyState(VK_SHIFT) != 0);
    Scene &scene = state->database.scenes.getScene(state->curScene);

    float screenX = (float)app->input.mouse.pos.x / (app->window.getWidth() - 1.0f);
    float screenY = (float)app->input.mouse.pos.y / (app->window.getHeight() - 1.0f);
    ml::Rayf ray = state->camera.getScreenRay(screenX, screenY);

    if (shift && button == ml::MouseButtonLeft)
    {
        selectedAgentIndex = -1;
        int agentIndex = 0;
        for (const Agent &agent : scene.agents)
        {
            ml::TriMeshAcceleratorBruteForcef accelerator;
            accelerator.build(state->renderer.agentHead().getTriMesh(), mat4f::translation(agent.headPos));

            ml::TriMeshRayAccelerator<float>::Intersection intersection;
            if (accelerator.intersect(ray, intersection))
                selectedAgentIndex = agentIndex;
            agentIndex++;
        }
    }

    if (shift && button == ml::MouseButtonRight)
    {
        ml::TriMeshRayAcceleratorf::Intersection intersection;
        UINT contactObjectIndex;
        vector<const ml::TriMeshAcceleratorBruteForcef*> vecPtr = ml::util::toVecConstPtr(objectAccelerators);
        if (ml::TriMeshRayAccelerator<float>::getFirstIntersection(ray, vecPtr, intersection, contactObjectIndex))
        {
            if (state->mode == VizzerModeAgentPlacement)
            {
                auto positionAgent = [&](Agent &agent)
                {
                    agent.supportObjectIndex = contactObjectIndex;
                    agent.supportPos = intersection.getSurfacePosition();

                    //
                    // Sitting versus standing height
                    //
                    float agentHeight = synthParams().agentHeightStanding;
                    if (contactObjectIndex != 0)
                        agentHeight = 0.7f;

                    agent.headPos = agent.supportPos + vec3f::eZ * agentHeight;
                };

                if (selectedAgentIndex == -1)
                {
                    //
                    // Create a new agent
                    //
                    Agent agent;
                    positionAgent(agent);

                    agent.gazeDir = vec3f::eX;
                    agent.rightDir = vec3f::eY;

                    agent.activity = activityList.front();

                    agent.scene = &scene;
                    scene.agents.push_back(agent);
                }
                else
                {
                    //
                    // Move the current agent
                    //
                    Agent &agent = scene.agents[selectedAgentIndex];
                    positionAgent(agent);
                }
            }

            if (state->mode == VizzerModeAgentOwnership && contactObjectIndex != 0)
            {
                for (Agent &a : scene.agents)
                {
                    int index = ml::util::findFirstIndex(a.boundObjectIndices, contactObjectIndex);
                    if (index != -1)
                    {
                        ml::util::removeSwap(a.boundObjectIndices, index);
                    }
                }
                if (selectedAgentIndex != -1 && contactObjectIndex != scene.agents[selectedAgentIndex].supportObjectIndex)
                    scene.agents[selectedAgentIndex].boundObjectIndices.push_back(contactObjectIndex);
            }

            dirty = true;
        }
    }
}

string AgentMode::helpText() const
{
    string text;
    if (state->mode == VizzerModeAgentPlacement)
    {
        text += "Left/right: rotate agent|";
        text += "Up/down: change agent's activity|";
        text += "Shift + left click: select agent|";
        text += "Shift + right click: move/create agent|";
        text += "R: reset agents";
    }
    if (state->mode == VizzerModeAgentOwnership)
    {
        text += "Shift + left click: select agent|";
        text += "Shift + right click: change ownership|";
        text += "R: reset agents|";
        text += "P: reset to proximity ownership";
    }
    return text;
}

vector< pair<string, RGBColor> > AgentMode::statusText() const
{
    const Scene &scene = state->database.scenes.getScene(state->curScene);

    vector< pair<string, RGBColor> > result;
    if (selectedAgentIndex == -1)
        result.push_back(std::make_pair(string("No agent selected"), RGBColor::White));
    else
    {
        const Agent &a = scene.agents[selectedAgentIndex];
        result.push_back(std::make_pair("Agent activity: " + a.activity, RGBColor(activityColors.at(a.activity))));
    }

    return result;
}
