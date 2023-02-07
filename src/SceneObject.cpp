#include "../headers/SceneObject.hpp"


namespace Geometry {

SceneObject::SceneObject() {

}


SceneObject::~SceneObject() {

}


glm::vec3 SceneObject::getPosition() {
	return Position_;
}


void SceneObject::setVisibility(bool vis) {
    isVisible_ = vis;
}


bool SceneObject::getVisibility() {
    return isVisible_;
}


AABB SceneObject::getAABB() {
	return boundingBox_;
}


glm::mat4 SceneObject::getModelMatrix() {
	return modelMatrix_;
}


void SceneObject::calculateModelMatrix() {
	modelMatrix_ = glm::rotate(glm::mat4(1.0f), RotAngle_, RotationVector_);
	modelMatrix_ = glm::scale(modelMatrix_, Scale_);
	modelMatrix_ = glm::translate(modelMatrix_, Position_);
}


}