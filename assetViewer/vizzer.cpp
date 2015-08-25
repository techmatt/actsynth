
#include "main.h"

#include <boost/filesystem.hpp>

const bool runUIDebugger = false;

void Vizzer::init(ml::ApplicationData& app)
{
    ParameterFile params;
    const string file = "../../synthParamsDefault.txt";
    if (!ml::util::fileExists(file)) {
      std::cerr << "Parameter file not found: " << file << endl;
      exit(-1);
      return;
    }
    params.addParameterFile(file);
#ifdef _DEBUG
    params.addParameterFile("../../synthParamsDefaultDebug.txt");
#endif

    app.graphics.castD3D11().toggleCullMode();

    params.overrideParameter("sceneList", "All");
    params.overrideParameter("skipTextureLoading", "false");

    initSynthParams(params);

    state.database.loadAll();
    //state.database.output("../../dump/");

    state.assets.init(state.database);

    state.agentPlacementMode.init(app, state);
    state.interactionMapMode.init(app, state);
    state.scoreMode.init(app, state);
    state.scanMode.init(app, state);
    state.resultsMode.init(app, state);

    loadScene(app, state.database.scenes.scenes.begin()->first);

    m_font.init(app.graphics, "Calibri");

    //state.camera = ml::Camera<float>(vec3f::origin, vec3f::eZ, vec3f::eX, 60.0f, (float)app.window.width() / app.window.height(), 0.01f, 1000.0f);
    state.camera = ml::Camera<float>("3.1007 -4.67105 6.07874 1 0 0 0 0.836286 -0.548293 0 0.548293 0.836286 0 0 1 60 1.39502 0.01 1000");
    state.camera.updateAspectRatio((float)app.window.getWidth() / app.window.getHeight());

    app.graphics.castD3D11().captureBackBuffer();

    state.renderer.init(app.graphics.castD3D11());

    if (runUIDebugger)
    {
        cout << "Run the UI under the debugger, then press any key" << endl;
        std::cin.get();
        state.ui.init("", "SceneSynthesis");
    }
    else
        state.ui.init("../UIWindowAssetViewer.exe", "SceneAssetViewer");

    string sceneListMessage = "sceneList ";
    for (const auto &scene : state.database.scenes.scenes)
        sceneListMessage = sceneListMessage + scene.first + "|";
    state.ui.sendMessage(sceneListMessage);

    state.mode = VizzerModeAgentPlacement;
    state.updateModeInterface();

    registerEventHandlers(app);
}

void Vizzer::registerEventHandlers(ml::ApplicationData& app)
{
    state.eventMap.registerEvent("terminate", [&](const vector<string> &params) {
        PostQuitMessage(0);
    });
    state.eventMap.registerEvent("loadScene", [&](const vector<string> &params) {
        if (params[1].size() > 0)
            loadScene(app, params[1]);
    });
    state.eventMap.registerEvent("loadResultScene", [&](const vector<string> &params) {
        if (params[1].size() > 0)
        {
            state.mode = modeFromName("Results");
            state.updateModeInterface();
            state.resultsMode.newScene(synthParams().synthesisDir + "results/" + params[1]);
        }
    });
    state.eventMap.registerEvent("renderResultsFolder", [&](const vector<string> &params) {
        if (params[1].size() > 0)
        {
            state.mode = modeFromName("Results");
            state.updateModeInterface();
            state.resultsMode.renderResultsFolder(synthParams().synthesisDir + "results/" + params[1]);
        }
    });
    state.eventMap.registerEvent("showBBoxes", [&](const vector<string> &params) {
        state.showBBoxes = ml::util::convertTo<bool>(params[1]);
    });
    state.eventMap.registerEvent("mode", [&](const vector<string> &params) {
        state.mode = modeFromName(params[1]);
        state.updateModeInterface();
    });
    state.eventMap.registerEvent("recategorize", [&](const vector<string> &params) {
        if (state.mode == VizzerModeInteractionMap)
            state.interactionMapMode.recategorizeModel(params[1]);
    });
    state.eventMap.registerEvent("SavePBRT", [&](const vector<string> &params) {
        state.modeInterface->renderPBRT(synthParams().PBRTDir);
    });
    state.eventMap.registerEvent("batchPBRTRendering", [&](const vector<string> &params) {
      batchPBRTRendering(app);
    });
    state.eventMap.registerEvent("savePBRTCamera", [&](const vector<string> &params) {
      savePBRTCamera(synthParams().synthesisDir + "results/pbrt_cameras/" + params[1]+".pbrt");
    });
}

void Vizzer::savePBRTCamera(const string& filename)
{
  ofstream file(filename);
  file << "LookAt " << state.camera.getEye() << ' ' << (state.camera.getEye() + state.camera.getLook()) << ' ' << state.camera.getUp() << endl;

  return;
}

void Vizzer::batchPBRTRendering(ml::ApplicationData& app)
{
  string PBRTDirBackup = synthParams().PBRTDir;

  boost::filesystem::path final_results_dir(synthParams().FinalResultsDir);
  for (boost::filesystem::directory_iterator it(final_results_dir), it_end; it != it_end; ++it) {
    string folder_name = it->path().filename().string();
    if (folder_name.find("final-") == std::string::npos || (folder_name.find("-human") == std::string::npos && folder_name.find("-prev") == std::string::npos && folder_name.find("-us") == std::string::npos))
      continue;

    if (!boost::filesystem::exists(PBRTDirBackup + "/" + folder_name))
      boost::filesystem::create_directories(PBRTDirBackup + "/" + folder_name);

    for (boost::filesystem::directory_iterator it_files(it->path()), it_files_end; it_files != it_files_end; ++it_files) {
      if(it_files->path().extension().string() != ".sss")
          continue;

      string pbrt_folder_name = PBRTDirBackup + "/" + folder_name + "/" + it_files->path().stem().string() + "/";
      if (boost::filesystem::exists(pbrt_folder_name))
        continue;

      cout << "PBRT exporting for " << it_files->path().string() << "...";

      boost::filesystem::create_directories(pbrt_folder_name);
      mutableSynthParams().PBRTDir = pbrt_folder_name;

      Scene scene;
      scene.load(it_files->path().string(), state.database);

      PBRT::CopyPBRTFiles();
      PBRT::PBRTPreamble(state.camera, synthParams().PBRTResolution, true);

      vector<vec3f> objectColors(scene.objects.size(), vec3f(1, 1, 1));
      state.renderer.renderPBRTObjects(state.assets, app.graphics, PBRT::PBRTFilename(), state.camera, scene, objectColors);

      //PBRT::PBRTFinalize();
      ofstream file;
      file.open(PBRT::PBRTFilename(), std::ofstream::out | std::ofstream::app);
      file << "WorldEnd" << endl;
      file.close();  

      cout << " done" << std::endl;
    }
  }

  mutableSynthParams().PBRTDir = PBRTDirBackup;

  return;
}

void Vizzer::loadScene(ml::ApplicationData& app, const string &sceneName)
{
    state.agentPlacementMode.closeScene();

    state.curScene = sceneName;
    cout << "Loading scene: " << state.curScene << endl;

    const Scene &scene = state.database.scenes.getScene(state.curScene);

    state.objectBBoxes.clear();
    for (const ObjectInstance &object : scene.objects)
    {
        mat4f cubeToWorld = object.worldBBox.getOBBToWorld() * mat4f::scale(0.5f) * mat4f::translation(vec3f(1.0f, 1.0f, 1.0f));
        state.objectBBoxes.push_back(ml::D3D11TriMesh(app.graphics, ml::Shapesf::wireframeBox(cubeToWorld, ml::vec4f(1.0f, 1.0f, 1.0f, 1.0f), 0.01f )));

        const bool visualizeContact = true;
        if (visualizeContact)
        {
            state.objectBBoxes.push_back(ml::D3D11TriMesh(app.graphics, ml::Shapesf::sphere(0.05f, object.contactPosition)));
            state.objectBBoxes.push_back(ml::D3D11TriMesh(app.graphics, ml::Shapesf::cylinder(object.contactPosition, object.contactPosition + object.contactNormal * 0.25f, 0.03f, 4, 10, ml::vec4f(0.9f, 0.1f, 0.1f, 1.0f))));
        }
    }

    state.agentPlacementMode.newScene();
    state.scoreMode.newScene();
}

void Vizzer::render(ml::ApplicationData& app)
{
    m_timer.frame();

    state.eventMap.dispatchEvents(state.ui);

    state.modeInterface->render();

    if (state.showBBoxes)
    {
        for (UINT bboxIndex = 0; bboxIndex < state.objectBBoxes.size(); bboxIndex++)
        {
            state.renderer.renderMesh(app.graphics.castD3D11(), state.camera.getCameraPerspective(), state.objectBBoxes[bboxIndex], vec3f(1.0f, 1.0f, 1.0f));
        }
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

    if (key == KEY_TAB)
    {
        state.mode = VizzerMode(((int)state.mode + 1) % (int)VizzerModeCount);
        state.updateModeInterface();
    }
}

void Vizzer::keyPressed(ml::ApplicationData& app, UINT key)
{
    state.modeInterface->keyPressed(key);

    //if (key == KEY_O) {
    //    std::ofstream file("camera.txt");
    //    file << state.InteractionMapMode.camera.toString();
    //}

    if (state.mode == VizzerModeInteractionMap) return;

    float distance = 0.2f;
    float theta = 2.5f;

    if (key == KEY_W) { state.camera.move(distance); }
    if (key == KEY_S) { state.camera.move(-distance); }
    if (key == KEY_A) { state.camera.strafe(-distance); }
    if (key == KEY_D) { state.camera.strafe(distance); }
    //if (key == KEY_E) { state.camera.jump(distance); }
    //if (key == KEY_Q) { state.camera.jump(-distance); }

    //
    // These can be accomplished by using the mouse, and are used by various mode interfaces.
    //
    //if (key == KEY_UP) { state.camera.lookUp(theta); }
    //if (key == KEY_DOWN) { state.camera.lookUp(-theta); }
    //if (key == KEY_LEFT) { state.camera.lookRight(-theta); }
    //if (key == KEY_RIGHT) { state.camera.lookRight(theta); }
}

void Vizzer::mouseDown(ml::ApplicationData& app, ml::MouseButtonType button)
{
    state.modeInterface->mouseDown(button);
}

void Vizzer::mouseWheel(ml::ApplicationData& app, int wheelDelta)
{
    state.modeInterface->mouseWheel(wheelDelta);

    if (state.mode == VizzerModeInteractionMap) return;

    const float distance = 0.002f;
    state.camera.move(distance * wheelDelta);
}

void Vizzer::mouseMove(ml::ApplicationData& app)
{
    state.modeInterface->mouseMove();

    if (state.mode == VizzerModeInteractionMap) return;

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
