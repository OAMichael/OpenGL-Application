#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "PotentialApp.hpp"


#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <tinygltf/tiny_gltf.h>



int main() {
    
    PotentialApp App;
    App.createWindow(3, 3, 1200, 900, "OpenGL");
    App.renderToWindow();
    App.terminateWindow();

    return 0;
}