
#include "main.h"

void ScanMode::init(ml::ApplicationData& _app, AppState &_state)
{
    app = &_app;
    state = &_state;

    activityList = Activity::activityList();
    activityColors = Activity::activityColors();

    dirty = false;
    scanIndex = 0;
    newScan();
    renderAgents = true;
}

void ScanMode::saveScan()
{
    if (dirty)
    {
        const Scan &scan = state->database.scans.getScan(state->database.scans.scanNames()[scanIndex]);
        cout << "Saving agents and alignment for " << scan.name << endl;

        const string scanDir = R"(V:\data\synthesis\scans\)";

        scan.saveAgentAnnotations(scanDir + scan.name + "_agents" + synthParams().scanAgentSuffix + ".arv");
        scan.saveAlignment(scanDir + scan.name + "_align.txt");

        dirty = false;
    }
}

void ScanMode::newScan()
{
    //
    // we just need to load any scene with the standard room geometry
    //
    const Scene &scene = state->database.scenes.scenes.begin()->second;
    const Scan &scan = state->database.scans.getScan(state->database.scans.scanNames()[scanIndex]);
    selectedAgentIndex = -1;

    objectAccelerators.clear();
    objectAccelerators.resize(1);

    //
    // ignore all objects except the base architecture
    //
    for (UINT objectIndex = 0; objectIndex < 1; objectIndex++)
    {
        const ObjectInstance &object = scene.objects[objectIndex];
        
        const ModelAssetData &ModelAssetData = state->assets.loadModel(app->graphics.castD3D11(), object.object.modelId);

        vector< pair<const ml::TriMeshf *, mat4f> > meshes;
        for (const auto &mesh : ModelAssetData.data)
            meshes.push_back(std::make_pair(&mesh.meshOriginal.getTriMesh(), object.modelToWorld * mat4f::scale(synthParams().architectureScale)));

        objectAccelerators[objectIndex].build(meshes);
    }

    scanMesh.load(app->graphics, scan.baseMesh);
}

void ScanMode::render()
{
    const Scene &scene = state->database.scenes.scenes.begin()->second;
    const Scan &scan = state->database.scans.getScan(state->database.scans.scanNames()[scanIndex]);

    state->renderer.renderObject(state->assets, app->graphics.castD3D11(), state->camera.getCameraPerspective() * mat4f::scale(synthParams().architectureScale), scene.objects[0], vec3f(1, 1, 1), false);

    if (renderAgents)
    {
        int agentIndex = 0;
        for (const Agent &a : scan.agents)
        {
            vec3f color = activityColors[a.activity];
            if (agentIndex != selectedAgentIndex)
                color *= 0.6f;
            state->renderer.renderAgent(app->graphics.castD3D11(), state->camera.getCameraPerspective(), a, color);
            agentIndex++;
        }
    }
    
    app->graphics.castD3D11().toggleCullMode();
    state->renderer.renderMesh(app->graphics.castD3D11(), state->camera.getCameraPerspective() * scan.transform, scanMesh, ml::vec3f(1, 1, 1));
    app->graphics.castD3D11().toggleCullMode();
}

void ScanMode::keyDown(UINT key)
{
    Scan &scan = state->database.scans.getScan(state->database.scans.scanNames()[scanIndex]);

    if (key == KEY_Z)
        renderSceneD3D(synthParams().synthesisDir + "scan.png");

    int scanDelta = 0;
    if (key == KEY_V) scanDelta = -1;
    if (key == KEY_B) scanDelta = 1;
    if (scanDelta != 0)
    {
        if (dirty)
        {
            saveScan();
            dirty = false;
        }
        scanIndex = ml::math::mod(scanIndex + scanDelta, state->database.scans.scanNames().size());
        newScan();
    }

    if (key == KEY_ESCAPE) selectedAgentIndex = -1;
    if (key == KEY_R)
    {
        scan.agents.clear();
        selectedAgentIndex = -1;
        dirty = true;
    }

    if (selectedAgentIndex == -1)
        return;

    if (key == KEY_BACKSPACE || key == KEY_DELETE)
    {
        std::swap(scan.agents[selectedAgentIndex], scan.agents.back());
        scan.agents.pop_back();
        selectedAgentIndex = -1;
        dirty = true;
        return;
    }
    
    Agent &agent = scan.agents[selectedAgentIndex];

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

void ScanMode::keyPressed(UINT key)
{
    Scan &scan = state->database.scans.getScan(state->database.scans.scanNames()[scanIndex]);

    if (key == KEY_X) renderAgents = !renderAgents;
    if (key == KEY_Y) scan.rotation -= 1.0f;
    if (key == KEY_U) scan.rotation += 1.0f;
    if (key == KEY_H) scan.translation.x -= 0.01f;
    if (key == KEY_J) scan.translation.x += 0.01f;
    if (key == KEY_N) scan.translation.y -= 0.01f;
    if (key == KEY_M) scan.translation.y += 0.01f;
    if (key == KEY_Y || key == KEY_U ||
        key == KEY_H || key == KEY_J ||
        key == KEY_N || key == KEY_M)
    {
        scan.transform = mat4f::translation(scan.translation) * mat4f::rotationZ(scan.rotation);
        dirty = true;
    }
}

void ScanMode::mouseDown(ml::MouseButtonType button)
{
    bool shift = (GetAsyncKeyState(VK_SHIFT) != 0);
    Scan &scan = state->database.scans.getScan(state->database.scans.scanNames()[scanIndex]);

    float screenX = (float)app->input.mouse.pos.x / (app->window.getWidth() - 1.0f);
    float screenY = (float)app->input.mouse.pos.y / (app->window.getHeight() - 1.0f);
    ml::Rayf ray = state->camera.getScreenRay(screenX, screenY);

    if (shift && button == ml::MouseButtonLeft)
    {
        selectedAgentIndex = -1;
        int agentIndex = 0;
        for (const Agent &agent : scan.agents)
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
            auto positionAgent = [&](Agent &agent)
            {
                agent.supportObjectIndex = contactObjectIndex;
                agent.supportPos = intersection.getSurfacePosition();

                //
                // Sitting versus standing height -> always assume sitting height for scans, except for bed
                //
                float agentHeight = synthParams().agentHeightSitting;

                if (agent.activity == "bed")
                    agentHeight = synthParams().agentHeightStanding;

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

                agent.scene = NULL;
                scan.agents.push_back(agent);
            }
            else
            {
                //
                // Move the current agent
                //
                Agent &agent = scan.agents[selectedAgentIndex];
                positionAgent(agent);
            }
            
            dirty = true;
        }
    }
}

string ScanMode::helpText() const
{
    string text;
    text += "Left/right: rotate agent|";
    text += "Up/down: change agent's activity|";
    text += "Shift + left click: select agent|";
    text += "Shift + right click: move/create agent|";
    text += "YUHJNM: change scan alignment|";
    text += "VB: scan selection|";
    text += "R: reset agents|";
    text += "X: toggle agent rendering";
    text += "Z: render scene";
    return text;
}

vector< pair<string, RGBColor> > ScanMode::statusText() const
{
    const Scan &scan = state->database.scans.getScan(state->database.scans.scanNames()[scanIndex]);

    vector< pair<string, RGBColor> > result;
    result.push_back(std::make_pair(string("Scan: ") + scan.name, RGBColor::Blue));
    if (selectedAgentIndex == -1)
        result.push_back(std::make_pair(string("No agent selected"), RGBColor::White));
    else
    {
        const Agent &a = scan.agents[selectedAgentIndex];
        result.push_back(std::make_pair("Agent activity: " + a.activity, RGBColor(activityColors.at(a.activity))));
    }
        
    return result;
}

void ScanMode::renderSceneD3D(const string &outputFilename) const
{
    Scan &scan = state->database.scans.getScan(state->database.scans.scanNames()[scanIndex]);

    cout << "rendering to " << outputFilename << endl;

    ml::Cameraf camera = state->camera;
    camera.updateAspectRatio(1.0f);

    ml::D3D11RenderTarget renderTarget;
    renderTarget.load(app->graphics, 1024, 1024);
    renderTarget.clear(vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    renderTarget.bind();

    app->graphics.castD3D11().toggleCullMode();
    state->renderer.renderMesh(app->graphics.castD3D11(), state->camera.getCameraPerspective() * scan.transform, scanMesh, ml::vec3f(1, 1, 1));
    app->graphics.castD3D11().toggleCullMode();

	Bitmap bmp;
    renderTarget.captureColorBuffer(bmp);
    ml::LodePNG::save(bmp, outputFilename);

    app->graphics.castD3D11().bindRenderTarget();
}
