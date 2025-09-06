#pragma once

#include "GL.h"


struct Input {
	static void keyCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods);
	static void mouseButtonCallback(GLFWwindow* pWindow, int button, int action, int mods);
	static void cursorPosCallback(GLFWwindow* pWindow, double xpos, double ypos);
};