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

    float Sensitivity_ = 0.15f;

    float Aspect_;
    float ZNear_;
    float ZFar_;

    float DirSpeedFactor_ = 1.0f;
    float RightSpeedFactor_ = 1.0f;

    float Yaw_ = -90.0f;
    float Pitch_ = 0.0f;
    float Fov_ = 90.0f;

    glm::mat4 View_;
    glm::mat4 Proj_;
    glm::mat4 Model_;

public:
    Camera(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& u, const glm::vec3& r) :
        Position_{ pos },
        Direction_{ dir },
        Up_{ u },
        Right_{ r }
    {};
    Camera() {};
    ~Camera() {};

    inline glm::vec3 getPosition() const { return Position_; }
    inline void setPosition(const glm::vec3& pos) { Position_ = pos; }

    inline glm::vec3 getDirection() const { return Direction_; }
    inline void setDirection(const glm::vec3& dir) { Direction_ = dir; }

    inline glm::vec3 getRight() const { return Right_; }
    inline void setRight(const glm::vec3& r) { Right_ = r; };

    inline glm::vec3 getUp() const { return Up_; }
    inline void setUp(const glm::vec3& u) { Up_ = u; }

    inline float getSensitivity() const { return Sensitivity_; }
    inline void setSensitivity(const float s) { Sensitivity_ = s; }

    inline float getAspect() const { return Aspect_; }
    inline void setAspect(const float a) { Aspect_ = a; }

    inline float getZNear() const { return ZNear_; }
    inline void setZNear(const float n) { ZNear_ = n; }

    inline float getZFar() const { return ZFar_; }
    inline void setZFar(const float f) { ZFar_ = f; }

    inline float getFov() const { return Fov_; }
    inline void setFov(const float f) { Fov_ = f; }

    inline glm::mat4 getView() const { return View_; }
    inline glm::mat4 getProj() const { return Proj_; }
    inline glm::mat4 getModel() const { return Model_; }

    inline void setDirSpeed(const float new_speed) { DirSpeedFactor_ = new_speed; }
    inline void setRightSpeed(const float new_speed) { RightSpeedFactor_ = new_speed; }

    inline float getYaw() const { return Yaw_; }
    inline void setYaw(const float yaw) { Yaw_ = yaw; }

    inline float getPitch() const { return Pitch_; }
    inline void setPitch(const float pitch) { Pitch_ = pitch; }

    inline void moveFront(const float delta) { Position_ += Direction_ * delta * DirSpeedFactor_; }
    inline void moveRight(const float delta) { Position_ += Right_ * delta * RightSpeedFactor_; }
        
    void updateVectors();
    void updateMatrices();
};

std::ostream& operator<<(std::ostream& os, const Camera& camera);

}
#endif