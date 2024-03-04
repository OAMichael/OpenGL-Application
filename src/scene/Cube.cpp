#include "Cube.hpp"


namespace Geometry {


Cube::Cube(const glm::vec3& pos) {
    Position_ = pos;

    boundingBox_ = { glm::vec3(-0.5f, -0.5f, -0.5f) + pos, glm::vec3(0.5f, 0.5f, 0.5f) + pos };

    calculateLocalModelMatrix();
}

}