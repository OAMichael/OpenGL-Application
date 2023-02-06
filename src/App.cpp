#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../headers/PotentialApp.hpp"


int main() {
    
    PotentialApp App;
    App.createWindow(3, 3, 1200, 900, "OpenGL");
    App.renderToWindow();
    App.terminateWindow();

    return 0;
}