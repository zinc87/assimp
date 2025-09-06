#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GL.h"



struct Camera {
	Camera() = default;

	glm::vec3 camPos = { 0.0f, 1.7f, 3.0f };
	glm::vec3 camFront = { 0.0f, 0.0f, -1.0f };
	glm::vec3 camUp = { 0.0f, 1.0f, 0.0f };

	float yaw = -90.0f;   // facing -Z
	float pitch = 0.0f;

	float moveSpeed = 10.0f;     // units/sec
	float mouseSens = 0.1f;     // deg/pixel

	bool  rotating = false;   // true while mouse button is held
	bool  firstMouse = true;
	double lastX = GLSetup::width/2, lastY = GLSetup::height/2; // init to window center

	void updateFrontYawPitch();

	static void CameraUpdate();
};