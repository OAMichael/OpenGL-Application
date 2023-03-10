#include "../headers/Camera.hpp"


namespace GeneralApp {

Camera::Camera() {

}


Camera::~Camera() {

}


Camera::Camera(const glm::vec3& pos, const glm::vec3& tar, const glm::vec3& u, const glm::vec3& r) { 
    Position_  = pos;
    Direction_ = glm::normalize(tar - pos);
    Up_ = u;
    Right_ = r;
    Target_ = tar;
}


glm::vec3 Camera::getPosition() const { 
    return Position_;  
}

void Camera::setPosition(const glm::vec3& pos) { 
    Position_ = pos;   
}


glm::vec3 Camera::getDirection() const { 
    return Direction_; 
}

void Camera::setDirection(const glm::vec3& dir) { 
    Direction_ = dir;  
}


glm::vec3 Camera::getRight() const { 
    return Right_; 
}

void Camera::setRight(const glm::vec3& r) { 
    Right_ = r; 
};


glm::vec3 Camera::getUp() const { 
    return Up_; 
}

void Camera::setUp(const glm::vec3& u) { 
    Up_ = u; 
}


glm::vec3 Camera::getTarget() const { 
    return Target_; 
}

void Camera::setTarget(const glm::vec3& t) { 
    Target_ = t; 
}


float Camera::getSensitivity() const { 
    return Sensitivity_; 
}

void Camera::setSensitivity(const float& s) { 
    Sensitivity_ = s; 
}


float Camera::getAspect() const { 
    return Aspect_; 
}

void Camera::setAspect(const float& a) { 
    Aspect_ = a; 
}


float Camera::getZNear() const { 
    return ZNear_; 
}

void Camera::setZNear(const float& n) { 
    ZNear_ = n; 
}


float Camera::getZFar() const { 
    return ZFar_; 
}

void Camera::setZFar(const float& f) { 
    ZFar_ = f; 
}


float Camera::getFov() const { 
    return Fov_; 
}

void Camera::setFov(const float& f) { 
    Fov_ = f; 
}


void Camera::setDirSpeed(const float& new_speed) { 
    DirSpeedFactor_ = new_speed; 
}

void Camera::setRightSpeed(const float& new_speed) { 
    RightSpeedFactor_ = new_speed; 
}


float Camera::getYaw() const { 
    return Yaw_; 
}

void Camera::setYaw(const float& yaw) { 
    Yaw_ = yaw; 
}


float Camera::getPitch() const { 
    return Pitch_; 
}

void Camera::setPitch(const float& pitch) { 
    Pitch_ = pitch; 
}


void Camera::moveFront(const float& delta) { 
    Position_ += Direction_ * delta * DirSpeedFactor_;
    Target_ = Position_ + Direction_;
}


void Camera::moveRight(const float& delta) { 
    Position_ += Right_ * delta * RightSpeedFactor_; 
    Target_ = Position_ + Direction_;
}


void Camera::updateVectors() {
    glm::vec3 Front;
    Front.x = cos(glm::radians(Yaw_)) * cos(glm::radians(Pitch_));
    Front.y = sin(glm::radians(Pitch_));
    Front.z = sin(glm::radians(Yaw_)) * cos(glm::radians(Pitch_));
    Direction_ = glm::normalize(Front);
    Right_ = glm::normalize(glm::cross(Direction_, glm::vec3(0.0, 1.0, 0.0)));
    Up_    = glm::normalize(glm::cross(Right_, Direction_));
    Target_ = Position_ + Direction_;
}


std::ostream& operator<<(std::ostream& os, const Camera& camera) {
    glm::vec3 position  = camera.getPosition();
    glm::vec3 direction = camera.getDirection();
    glm::vec3 right     = camera.getRight();
    glm::vec3 up        = camera.getUp();
    os << "Position = ("  << position.x  << ", " << position.y  << ", " << position.z  << ");   " << 
          "Direction = (" << direction.x << ", " << direction.y << ", " << direction.z << ");   " <<
          "Right = ("     << right.x     << ", " << right.y     << ", " << right.z     << ");   " <<
          "Up = ("        << up.x        << ", " << up.y        << ", " << up.z        << ")";

    return os;
}

}