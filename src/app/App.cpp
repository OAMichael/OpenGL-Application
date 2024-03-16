#include "PotentialApp.hpp"


int main() {
    
    PotentialApp app;
    GeneralApp::WindowCreateInfo wci;
    wci.winWidth = 1200;
    wci.winHeight = 900;
    wci.name = "OpenGL";

    app.createWindow(wci);
    app.renderToWindow();
    app.terminateWindow();

    return 0;
}