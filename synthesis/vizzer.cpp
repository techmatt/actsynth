
#include "main.h"

const bool runUIDebugger = false;
const string testScene = "wssScenes.scene00031";

void Vizzer::updateScanVisualization(ml::ApplicationData& app)
{
    vector<vec4f> vertexColors(state.targetScan->transformedMesh.getVertices().size(), vec4f(0.5, 0.5, 0.5, 1.0f));
    state.targetScanMesh.updateColors(app.graphics, vertexColors);
}

void Vizzer::init(ml::ApplicationData& app)
{
    ParameterFile params;
    std::string cwd = ml::util::workingDirectory();

    params.addParameterFile("../../synthParamsDefault.txt");
#ifdef _DEBUG
    params.addParameterFile("../../synthParamsDefaultDebug.txt");
#endif

    params.overrideParameter("skipTextureLoading", "false");

    initSynthParams(params);

    app.graphics.castD3D11().toggleCullMode();
    
    state.database.loadAll();
    //state.database.output("../../dump/");

    //
    // Special case for big dining scene...only allow one smaller table type
    //
    if (ml::util::startsWith(synthParams().scanName, "dining-large"))
        for (auto &m : state.database.models.models)
        {
            //if (m.second.categoryName == "Table" && !ml::util::contains(m.first, "ca000b9d32450bc4eb6d0fd624ec3fc"))
            if (m.second.categoryName == "Table" && !ml::util::contains(m.first, "8d533bfd684499c698d5fc0473d00a1c"))
                m.second.blacklisted = true;
        }

    state.assets.init(state.database);

    state.targetScan = &state.database.scans.getScan(synthParams().scanName);

    state.targetScanMesh.load(app.graphics, state.targetScan->transformedMesh);

    state.sceneTemplate.init(state.database, *state.targetScan);

    state.synth.start(state.database, app.graphics.castD3D11(), state.assets, state.sceneTemplate, "wss.room01", 0.0254f * synthParams().architectureScale);

    updateScanVisualization(app);

    state.fieldMode.init(app, state);

    m_font.init(app.graphics, "Calibri");

    state.camera = ml::Cameraf("3.1007 -4.67105 6.07874 1 0 0 0 0.836286 -0.548293 0 0.548293 0.836286 0 0 1 60 1.39502 0.01 1000");
    state.camera.updateAspectRatio((float)app.window.getWidth() / app.window.getHeight());

    state.renderer.init(app.graphics.castD3D11());

    if (runUIDebugger)
    {
        cout << "Run the UI under the debugger, then press any key" << endl;
        std::cin.get();
        state.ui.init("", "SceneSynthesis");
    }
    else
        state.ui.init("../UIWindowSynth.exe", "SceneSynthesis");

    state.mode = VizzerModeField;

    state.updateModeInterface();

    registerEventHandlers(app);
}

void Vizzer::registerEventHandlers(ml::ApplicationData& app)
{
    state.eventMap.registerEvent("terminate", [&](const vector<string> &params) {
        PostQuitMessage(0);
    });
    state.eventMap.registerEvent("showBBoxes", [&](const vector<string> &params) {
        state.showBBoxes = ml::util::convertTo<bool>(params[1]);
    });
    state.eventMap.registerEvent("showScan", [&](const vector<string> &params) {
        state.showScan = ml::util::convertTo<bool>(params[1]);
    });
    state.eventMap.registerEvent("showPlanes", [&](const vector<string> &params) {
        state.showPlanes = ml::util::convertTo<bool>(params[1]);
    });
    state.eventMap.registerEvent("showScene", [&](const vector<string> &params) {
        state.showScene = ml::util::convertTo<bool>(params[1]);
    });
    state.eventMap.registerEvent("showScanColumns", [&](const vector<string> &params) {
        state.showScanColumns = ml::util::convertTo<bool>(params[1]);
        state.fieldMode.updateColumns();
    });
    state.eventMap.registerEvent("showHeatmap", [&](const vector<string> &params) {
        state.showHeatmap = ml::util::convertTo<bool>(params[1]);
    });
    state.eventMap.registerEvent("showAgents", [&](const vector<string> &params) {
        state.showAgents = ml::util::convertTo<bool>(params[1]);
    });
    state.eventMap.registerEvent("showCollisionMesh", [&](const vector<string> &params) {
        state.showCollisionMesh = ml::util::convertTo<bool>(params[1]);
    });
    state.eventMap.registerEvent("mode", [&](const vector<string> &params) {
        state.mode = modeFromName(params[1]);
        state.updateModeInterface();
    });

    state.eventMap.registerEvent("viewMax", [&](const vector<string> &params) {
        state.fieldMode.viewMax(-1);
    });
}

void Vizzer::render(ml::ApplicationData& app)
{
    m_timer.frame();

    state.eventMap.dispatchEvents(state.ui);

    state.modeInterface->render();

    if (state.showAgents)
    {
        int agentIndex = 0;
        for (const Agent &a : state.synth.scene.agents)
        {
            vec3f color = Activity::activityColors()[a.activity];
            state.renderer.renderAgent(app.graphics.castD3D11(), state.camera.getCameraPerspective(), a, color);
            agentIndex++;
        }

        if (state.synth.activityBlockers.size() > 0 && state.activityBlockersMesh.getTriMesh().getVertices().size() == 0)
        {
            vector<ml::TriMeshf> meshes;

            for (const ActivityBlocker &blocker : state.synth.activityBlockers)
            {
                //meshes.push_back(ml::Shapesf::wireframeBox(obb.getOBBToWorld(), vec4f(1.0f, 0.5f, 0.5f, 0.0f), 0.02f));
                meshes.push_back(ml::Shapesf::box(blocker.obb, vec4f(1.0f, 0.5f, 0.5f, 0.0f)));
            }

            state.activityBlockersMesh.load(app.graphics, ml::meshutil::createUnifiedMesh(meshes));
        }
        state.renderer.renderMesh(app.graphics.castD3D11(), state.camera.getCameraPerspective(), state.activityBlockersMesh);
    }

    if (state.showScan)
    {
        state.renderer.renderMesh(app.graphics.castD3D11(), state.camera.getCameraPerspective(), state.targetScanMesh);
    }

    if (state.showPlanes)
    {
        const GeometricRepresentation &g = state.sceneTemplate.geometry;
        //cout << "showing planes 1" << endl;
        for (const Plane &p : g.planes)
        {
            //cout << "showing planes 2" << endl;
            const float val = p.id / float(g.planes.size());
            const vec3f color = ml::BaseImageHelper::convertHSVtoRGB(vec3f(240.0f* val, 1.0f, 0.5f));

            //for (int vIndex = 0; vIndex < p.convex_hull.size(); vIndex++)
            //{
            //    const vec3f &v0 = p.convex_hull[vIndex + 0];
            //    const vec3f &v1 = p.convex_hull[(vIndex + 1) % p.convex_hull.size()];
            //    state.renderer.renderSphere(app.graphics.castD3D11(), state.camera.cameraPerspective(), v0, 0.015f, color);
            //    state.renderer.renderCylinder(app.graphics.castD3D11(), state.camera.cameraPerspective(), v0, v1, color);
            //}

            for (int vIndex = 0; vIndex < p.simplified_hull.size(); vIndex++)
            {
                const vec3f &v0 = p.simplified_hull[vIndex + 0];
                const vec3f &v1 = p.simplified_hull[(vIndex + 1) % p.simplified_hull.size()];
                state.renderer.renderSphere(app.graphics.castD3D11(), state.camera.getCameraPerspective(), v0, 0.015f, color);
                state.renderer.renderCylinder(app.graphics.castD3D11(), state.camera.getCameraPerspective(), v0, v1, color);
            }
        }
    }

    if (state.showHeatmap)
    {
        
    }

    vector< pair<string, RGBColor> > lines;

    lines.push_back(std::make_pair("FPS: " + to_string(m_timer.framesPerSecond()), RGBColor::Red));
    lines.push_back(std::make_pair("[tab] Mode: " + modeName(state.mode), RGBColor::Red));

    for (const auto &line : state.modeInterface->statusText())
        lines.push_back(line);

    m_font.drawStrings(app.graphics, lines, ml::vec2i(10, 5), 24.0f, 27);
}

void Vizzer::resize(ml::ApplicationData& app)
{
    state.camera.updateAspectRatio((float)app.window.getWidth() / app.window.getHeight());
    //m_font.reset(app.graphics);
}

void Vizzer::keyDown(ml::ApplicationData& app, UINT key)
{
    state.modeInterface->keyDown(key);

    if (key == KEY_F) app.graphics.castD3D11().toggleWireframe();

    if (key == KEY_G)
    {
        if (GetAsyncKeyState(VK_SHIFT) != 0)
            state.synth.synthesize();
        state.synth.step();
        state.fieldMode.viewMax(-1);
        //state.synth.outputState("../../cache/synthState.html");
    }

    if (key == KEY_Z)
    {
#ifndef _DEBUG
        set<string> neededCategories;
        for (const Agent &scanAgent : state.sceneTemplate.debugScan->agents)
        {
            const Activity &a = state.database.activities.getActivity(scanAgent.activity);
            for (const auto &category : a.categoryDistributions)
                neededCategories.insert(category.first);
        }
        state.assets.loadAllModels(app.graphics.castD3D11(), neededCategories);
#endif

        const UINT maxSynthesisResults = 100000;
        cout << "Synthesizing " << maxSynthesisResults << " scenes in parallel" << endl;
        
        string resultsDir = synthParams().synthesisDir + "results/" + synthParams().scanName + "/";
        ml::util::makeDirectory(resultsDir);

#pragma omp parallel for num_threads(7)
        for (int i = 0; i < maxSynthesisResults; i++)
        {
            Timer t;
            Synthesizer synth;
            synth.start(state.database, app.graphics.castD3D11(), state.assets, state.sceneTemplate, "wss.room01", 0.0254f * synthParams().architectureScale);
            synth.synthesize();

            string earlyTermination = synth.earlyTermination ? "terminated_" : "";

            const double d = ml::util::randomUniform();
            const UINT64 hash = ml::util::hash64(d + i);
            const string hashString = ml::util::encodeBytes((const unsigned char *)&hash, sizeof(UINT64)) + ".sss";
            synth.scene.save(resultsDir + earlyTermination + hashString);
            cout << "Scene " << i << " done, synthesis time: " << t.getElapsedTime() << "s" << endl;
        }
    }

    if (key == KEY_TAB)
    {
        state.mode = VizzerMode(((int)state.mode + 1) % (int)VizzerModeCount);
        state.updateModeInterface();
    }

    if (key == KEY_R)
    {
        state.synth = Synthesizer();
        state.synth.start(state.database, app.graphics.castD3D11(), state.assets, state.sceneTemplate, "wss.room01", 0.0254f * synthParams().architectureScale);
    }

    int deltaScanObject = 0;
    if (key == KEY_N) deltaScanObject = -1;
    if (key == KEY_M) deltaScanObject = 1;
}

void Vizzer::keyPressed(ml::ApplicationData& app, UINT key)
{
    state.modeInterface->keyPressed(key);

    float distance = 0.2f;
    float theta = 2.5f;

    if (key == KEY_W) { state.camera.move(distance); }
    if (key == KEY_S) { state.camera.move(-distance); }
    if (key == KEY_A) { state.camera.strafe(-distance); }
    if (key == KEY_D) { state.camera.strafe(distance); }
}

void Vizzer::mouseDown(ml::ApplicationData& app, ml::MouseButtonType button)
{
    state.modeInterface->mouseDown(button);
}

void Vizzer::mouseWheel(ml::ApplicationData& app, int wheelDelta)
{
    state.modeInterface->mouseWheel(wheelDelta);

    const float distance = 0.002f;
    state.camera.move(distance * wheelDelta);
}

void Vizzer::mouseMove(ml::ApplicationData& app)
{
    state.modeInterface->mouseMove();

    bool shift = (GetAsyncKeyState(VK_SHIFT) != 0);
    if (shift)
        return;

    const float distance = 0.01f;
    const float theta = 0.25f;

    ml::vec2i posDelta = app.input.mouse.pos - app.input.prevMouse.pos;

    if (app.input.mouse.buttons[ml::MouseButtonRight])
    {
        state.camera.strafe(distance * posDelta.x);
        state.camera.jump(-distance * posDelta.y);
    }

    if (app.input.mouse.buttons[ml::MouseButtonLeft])
    {
        state.camera.lookRight(-theta * posDelta.x);
        state.camera.lookUp(theta * posDelta.y);
    }
}
