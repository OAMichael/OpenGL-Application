#ifndef ISCENEOBJECT_HPP
#define ISCENEOBJECT_HPP

#include <iostream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#define INVALID_SCENE_OBJECT 0xFFFFFFFFFFFFFFFFu

namespace SceneResources {

struct AABB {
	glm::vec3 minCorner;
	glm::vec3 maxCorner;
};

struct SceneHandle {
	uint64_t nativeHandle = INVALID_SCENE_OBJECT;

	bool operator==(const SceneHandle& other) const { return nativeHandle == other.nativeHandle; }
};

class ISceneObject {

protected:
	glm::vec3 Position_ = glm::vec3(0.0f);

	glm::quat Rotation_ = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	glm::vec3 Scale_ = glm::vec3(1.0f);

    bool isVisible_ = true;

    AABB boundingBox_;

    glm::mat4 modelMatrix_;


	void calculateLocalModelMatrix();

public:
	SceneHandle handle;

	ISceneObject() {};
	virtual ~ISceneObject() = 0;

	void setPosition(glm::vec3 pos) { Position_ = pos; }
	glm::vec3 getPosition() { return Position_; }
	void setRotation(glm::quat rot) { Rotation_ = rot; }
	void setScale(glm::vec3 scl) { Scale_ = scl; }
	void setVisibility(bool vis) { isVisible_ = vis; }
	bool getVisibility() { return isVisible_; }
	AABB getAABB() { return boundingBox_; }
	glm::mat4 getLocalModelMatrix() { return modelMatrix_; }
};

}

template<>
struct std::hash<SceneResources::SceneHandle>
{
	std::size_t operator()(const SceneResources::SceneHandle& handle) const noexcept
	{
		return std::hash<uint64_t>{}(handle.nativeHandle);
	}
};

#endif