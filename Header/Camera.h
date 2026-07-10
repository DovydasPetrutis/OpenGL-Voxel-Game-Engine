#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera 
{
private:
	float fov = 45.0f;
public:
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraRight;
	glm::vec3 cameraUp;
	float cameraSpeed;
	float yaw = 0.0f;
	float pitch = 0.0f;
	
	void init(glm::vec3 pos, glm::vec3 target, float speed, float fov);
	Camera();

	void updateCameraRight();
	void updateCameraUp();
	void setFov(float fov);
	float getFov();
};