#include "GLSLversion.h"

layout(location = 0) in vec3 inPos;

layout(location = 0) out vec3 outPos;


uniform mat4 proj;
uniform mat4 view;


void main() {
    outPos = inPos;
    gl_Position =  proj * view * vec4(outPos, 1.0);
}
