#include "ISceneObject.hpp"


namespace Geometry {

ISceneObject::~ISceneObject() {

}

glm::vec3 ISceneObject::getPosition() {
	return Position_;
}


void ISceneObject::setVisibility(bool vis) {
    isVisible_ = vis;
}


bool ISceneObject::getVisibility() {
    return isVisible_;
}


AABB ISceneObject::getAABB() {
	return boundingBox_;
}


glm::mat4 ISceneObject::getLocalModelMatrix() {
	return modelMatrix_;
}


void ISceneObject::calculateLocalModelMatrix() {
	modelMatrix_ = glm::translate(glm::mat4(1.0f), Position_) * glm::toMat4(Rotation_) * glm::scale(glm::mat4(1.0f), Scale_);
}


}