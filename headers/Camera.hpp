#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>


namespace GeneralApp {

class Camera  { 
private:
    glm::vec3 Position_;
    glm::vec3 Direction_;
    glm::vec3 Right_;
    glm::vec3 Up_;

    glm::vec3 Target_;

    float Sensitivity_ = 0.15f;

    float Aspect_;
    float ZNear_;
    float ZFar_;

    float DirSpeedFactor_ = 1.0f;
    float RightSpeedFactor_ = 1.0f;

    float Yaw_ = -90.0f;
    float Pitch_ = 0.0f;
    float Fov_ = 90.0f;

public:
    Camera();
    Camera(const glm::vec3& pos, const glm::vec3& tar, const glm::vec3& u, const glm::vec3& r);
    ~Camera();

    glm::vec3 getPosition() const;
    void setPosition(const glm::vec3& pos);

    glm::vec3 getDirection() const;
    void setDirection(const glm::vec3& dir);
    
    glm::vec3 getRight() const;
    void setRight(const glm::vec3& r);
    
    glm::vec3 getUp() const;
    void setUp(const glm::vec3& u);

    glm::vec3 getTarget() const;
    void setTarget(const glm::vec3& t);

    float getSensitivity() const;
    void setSensitivity(const float& s);

    float getAspect() const;
    void setAspect(const float& a);

    float getZNear() const;
    void setZNear(const float& n);

    float getZFar() const;
    void setZFar(const float& f);
    
    float getFov() const;
    void setFov(const float& f);

    void setDirSpeed(const float& new_speed);
    void setRightSpeed(const float& new_speed);

    float getYaw() const;
    void setYaw(const float& yaw);

    float getPitch() const;
    void setPitch(const float& pitch);
    
    void moveFront(const float& delta);
    void moveRight(const float& delta);
        
    void updateVectors();
};

std::ostream& operator<<(std::ostream& os, const Camera& camera);

}
#endif