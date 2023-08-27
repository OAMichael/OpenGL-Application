#include "../headers/Cube.hpp"

#include "../3rdparty/stb_image.h"


namespace Geometry {

Cube::Cube() {

}


Cube::~Cube() {

}


Cube::Cube(const glm::vec3& pos) {
    Position_ = pos;

    boundingBox_ = { glm::vec3(-0.5f, -0.5f, -0.5f) + pos, glm::vec3(0.5f, 0.5f, 0.5f) + pos };

    calculateLocalModelMatrix();
}


const std::vector<float>& Cube::getVertices() const {
	return vertices_;
}

}