#include "Camera.hpp"


namespace GeneralApp {

void Camera::updateVectors() {
    glm::vec3 Front;
    Front.x = cos(glm::radians(Yaw_)) * cos(glm::radians(Pitch_));
    Front.y = sin(glm::radians(Pitch_));
    Front.z = sin(glm::radians(Yaw_)) * cos(glm::radians(Pitch_));
    Direction_ = glm::normalize(Front);
    Right_ = glm::normalize(glm::cross(Direction_, glm::vec3(0.0, 1.0, 0.0)));
    Up_    = glm::normalize(glm::cross(Right_, Direction_));
}


void Camera::updateMatrices() {
    View_ = glm::lookAt(Position_, Position_ + Direction_, Up_);
    Proj_ = glm::perspective(glm::radians(Fov_), Aspect_, ZNear_, ZFar_);
    Model_ = glm::mat4(1.0f);
}


std::ostream& operator<<(std::ostream& os, const Camera& camera) {
    glm::vec3 position  = camera.getPosition();
    glm::vec3 direction = camera.getDirection();
    glm::vec3 right     = camera.getRight();
    glm::vec3 up        = camera.getUp();
    os << "Position = (" << position.x  << ", " << position.y  << ", " << position.z  << ");   " << 
          "Direction =(" << direction.x << ", " << direction.y << ", " << direction.z << ");   " <<
          "Right =    (" << right.x     << ", " << right.y     << ", " << right.z     << ");   " <<
          "Up =       (" << up.x        << ", " << up.y        << ", " << up.z        << ")";

    return os;
}

}