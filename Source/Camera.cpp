#include "Camera.h"
void Camera::init(glm::vec3 pos, glm::vec3 target, float speed, float fov)
{
	this->fov = fov;
	cameraSpeed = speed;
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraPos = pos;
	cameraFront = glm::normalize(target);

	pitch = glm::degrees(asin(cameraFront.y));
	yaw = glm::degrees(atan2(cameraFront.z, cameraFront.x));

	cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
	cameraUp = glm::cross(cameraRight, cameraFront);
}
Camera::Camera()
{
}


void Camera::updateCameraRight()
{
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
}

void Camera::updateCameraUp()
{
	cameraUp = glm::cross(cameraRight, cameraFront);
}

void Camera::setFov(float fov)
{
	this->fov = fov;
}

float Camera::getFov()
{
	return fov;
}

