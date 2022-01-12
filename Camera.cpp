#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = glm::normalize(cameraUp);
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::cross(cameraFrontDirection, cameraUp);

        //TODO - Update the rest of camera parameters

    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        switch (direction) {
        case MOVE_FORWARD:
            cameraPosition += cameraFrontDirection * speed;
            break;

        case MOVE_BACKWARD:
            cameraPosition -= cameraFrontDirection * speed;
            break;

        case MOVE_RIGHT:
            cameraPosition += cameraRightDirection * speed;
            break;

        case MOVE_LEFT:
            cameraPosition -= cameraRightDirection * speed;
            break;
        }

        cameraTarget = cameraPosition + cameraFrontDirection;
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {

        cameraFrontDirection = glm::normalize(glm::rotate(glm::mat4(1.0f), yaw, cameraUpDirection) * glm::rotate(glm::mat4(1.0f), pitch, cameraRightDirection) * glm::vec4(cameraFrontDirection, 0));
        cameraTarget = cameraFrontDirection + cameraPosition;
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));

    }

    glm::vec3 Camera::getCameraPos() {
        return this->cameraPosition;
    }

    void Camera::setCameraPos(glm::vec3 cameraPos) {
        this->cameraPosition = cameraPos;
    }

    glm::vec3 Camera::getCameraFrontDirection() {
        return this->cameraFrontDirection;
    }

    void Camera::setCameraFrontDirection(glm::vec3 cameraPos) {
        this->cameraFrontDirection = cameraPos;
    }

    glm::vec3 Camera::getCameraTarget() {
        return this->cameraTarget;
    }

    void Camera::setCameraTarget(glm::vec3 cameraPos) {
        this->cameraTarget = cameraPos;
    }

    glm::vec3 Camera::getCameraUpDirection() {
        return this->cameraUpDirection;
    }

    void Camera::setCameraUpDirection(glm::vec3 cameraPos) {
        this->cameraUpDirection = cameraPos;
    }
    void Camera::computeCameraUpDirection() {
        this->cameraFrontDirection = cameraTarget - cameraPosition;

    }

    void Camera::computeCameraRightDirection() {
        this->cameraRightDirection = glm::cross(cameraFrontDirection, cameraUpDirection);
    }

}