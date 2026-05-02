#include "camera.h"

Camera::Camera()
{
	update();
}

Camera::Camera(const glm::vec3& position, const glm::vec3& target)
{
	pos = position;
	const glm::vec3 direction = glm::normalize(target - position);

	pitch = glm::degrees(asin(direction.y));
	yaw = glm::degrees(atan2(direction.z, direction.x));

	update();
}

void Camera::assignWindowObject(GLFWwindow* window)
{
	glfwSetWindowUserPointer(window, this);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Camera::setAspectRatio(float ratio)
{
	aspectRatio = ratio;
}

void Camera::handleInput(GLFWwindow* window, float deltaTime)
{
	// WASD
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		pos += front * moveSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		pos -= front * moveSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		pos -= right * moveSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		pos += right * moveSpeed * deltaTime;

	// Up and down
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		pos += worldUp * moveSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		pos -= worldUp * moveSpeed * deltaTime;

	// mouse
	double mouseX = 0.0, mouseY = 0.0;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	static bool firstMove = true;
	if (firstMove)
	{
		std::cout << mouseX << " :: " << mouseY << std::endl;
		lastMouseX = static_cast<float>(mouseX);
		lastMouseY = static_cast<float>(mouseY);
		firstMove = false;
		return;
	}

	float xOffset = (float)(mouseX - lastMouseX) * sensitivity;
	float yOffset = (float)(lastMouseY - mouseY) * sensitivity; // inverted Y
	
	lastMouseX = static_cast<float>(mouseX);
	lastMouseY = static_cast<float>(mouseY);

	yawDesired += xOffset;
	pitchDesired += yOffset;
	pitchDesired = glm::clamp(pitchDesired, -89.0f, 89.0f); // prevent flipping

	// Smoothly interpolate current toward desired:
	yaw += (yawDesired - yaw) * damping * deltaTime;
	pitch += (pitchDesired - pitch) * damping * deltaTime;

	update();
}

void Camera::update()
{
	glm::vec3 _front;
	_front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	_front.y = sin(glm::radians(pitch));
	_front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	front = glm::normalize(_front);
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}
