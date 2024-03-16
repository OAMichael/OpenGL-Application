#include "PotentialApp.hpp"


/*
static int32_t engine_handle_input(struct android_app* app,
                                   AInputEvent* event) {
  auto* engine = (struct engine*)app->userData;
  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
    engine->animating = 1;
    engine->state.x = AMotionEvent_getX(event, 0);
    engine->state.y = AMotionEvent_getY(event, 0);
    return 1;
  }
  return 0;
}
*/

void android_main(struct android_app* aapp) {
    PotentialApp app;
    GeneralApp::WindowCreateInfo wci;
    wci.app = aapp;
    wci.name = "OpenGL";

    app.createWindow(wci);
    app.renderToWindow();
    app.terminateWindow();
}
