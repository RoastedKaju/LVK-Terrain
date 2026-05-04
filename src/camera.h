#pragma once

#include <iostream>
#include <lvk/LVK.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <GLFW/glfw3.h>

class Camera
{
public:
	Camera();
	Camera(const glm::vec3& position, const glm::vec3& target);

	inline glm::mat4 getViewMatrix() const { return glm::lookAt(pos, pos + front, up); }
	inline glm::mat4 getProjMatrix() const { return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane); }
	inline glm::vec3 getCameraPosition() const { return pos; }

	void assignWindowObject(GLFWwindow* window);
	void setAspectRatio(float ratio);
	void handleInput(GLFWwindow* window, float deltaTime);

private:
	void update();

	float fov = 45.0f;
	float aspectRatio = 1.7777f;
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;

	glm::vec3 pos = glm::vec3(0.0f);
	glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::vec3(0.0f);
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

	float yaw = 0.0f;
	float pitch = 0.0f;

	float moveSpeed = 150.5f;
	float sensitivity = 0.35f;
	float lastMouseX = 0.0f;
	float lastMouseY = 0.0f;
	float yawDesired = 0.0f;
	float pitchDesired = 0.0f;
	float damping = 15.0f;

	bool cursorVisible = true;
};