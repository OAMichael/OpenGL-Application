#ifndef ISCENEOBJECT_HPP
#define ISCENEOBJECT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Geometry {

struct AABB {
	glm::vec3 minCorner;
	glm::vec3 maxCorner;
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
	glm::vec3 getPosition();
	void setPosition(glm::vec3 pos) { Position_ = pos; }

	void setRotation(glm::quat rot) { Rotation_ = rot; }
	void setScale(glm::vec3 scl) { Scale_ = scl; }

	void setVisibility(bool vis);

    bool getVisibility();

	ISceneObject() {};

    virtual ~ISceneObject() = 0;

    AABB getAABB();

    glm::mat4 getLocalModelMatrix();
};

}

#endif