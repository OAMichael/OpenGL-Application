#include "ISceneObject.hpp"


namespace SceneResources {

ISceneObject::~ISceneObject() {}

void ISceneObject::calculateLocalModelMatrix() {
	modelMatrix_ = glm::translate(glm::mat4(1.0f), Position_) * glm::toMat4(Rotation_) * glm::scale(glm::mat4(1.0f), Scale_);
}

}