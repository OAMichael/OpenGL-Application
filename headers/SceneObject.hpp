#ifndef SCENEOBJECT_HPP
#define SCENEOBJECT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace Geometry {

struct AABB {
	glm::vec3 minCorner;
	glm::vec3 maxCorner;
};


class SceneObject {

protected:
	glm::vec3 Position_ = glm::vec3(0.0f);

	glm::vec3 RotationVector_ = glm::vec3(0.0f, 0.0f, 1.0f);
	float RotAngle_ = 0.0f;

	glm::vec3 Scale_ = glm::vec3(1.0f);


    bool isVisible_ = true;

    AABB boundingBox_;

    glm::mat4 modelMatrix_;

public:
	glm::vec3 getPosition();

	void setVisibility(bool vis);

    bool getVisibility();

    SceneObject();

    virtual ~SceneObject() = 0;

    AABB getAABB();

    void calculateModelMatrix();
    glm::mat4 getModelMatrix();
};

}

#endif