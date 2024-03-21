#include "PotentialApp.hpp"


void android_main(struct android_app* aapp) {
    PotentialApp app;
    GeneralApp::WindowCreateInfo wci;
    wci.app = aapp;
    wci.name = "OpenGL";

    app.createWindow(wci);
    app.renderToWindow();
    app.terminateWindow();
}
