
//
// main.h
//
// Standard header for synthesis project
//

#include "mLibInclude.h"
#include "libsynth.h"

using ml::D3D11TriMesh;
using ml::D3D11ShaderManager;

#include "../synthesisCommon/assetManager.h"
#include "../synthesisCommon/assetRenderer.h"
#include "../synthesisCommon/eventMap.h"

struct AppState;

class VizzerModeInterface
{
public:
    virtual void render() {}
    virtual void renderPBRT(const std::string& path) {}
    virtual void keyDown(UINT key) {}
    virtual void keyPressed(UINT key) {}
    virtual void mouseDown(ml::MouseButtonType button) {}
    virtual void mouseMove() {}
    virtual void mouseWheel(int wheelDelta) {}
    virtual vector< pair<string, RGBColor> > statusText() const
    {
        return vector< pair<string, RGBColor> >();
    }
    virtual string helpText() const
    {
        return string("No help text");
    }
};

#include "agentMode.h"
#include "interactionMapMode.h"
#include "scoreMode.h"
#include "scanMode.h"
#include "resultsMode.h"

#include "appState.h"
#include "vizzer.h"
