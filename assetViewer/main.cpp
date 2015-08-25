
#include "main.h"

int main(int argc, const char** argv)
{
    Vizzer callback;
    ml::ApplicationWin32 app(NULL, 800, 600, "Vizzer", ml::GraphicsDeviceTypeD3D11, callback);
    app.messageLoop();

    return 0;
}
