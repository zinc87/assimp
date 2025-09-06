#include "GL.h"
#include "Input.h"
#include "camera.h"


extern Camera cam1;

void Input::keyCallback(GLFWwindow * pWindow, int key, int scancode, int action, int mods)
{
    // Close the app on ESC key pressed event
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_ESCAPE)
        {
            glfwSetWindowShouldClose(pWindow, GL_TRUE);
        }
        if (key == GLFW_KEY_ENTER) {
            GLSetup::matToggle = !GLSetup::matToggle;
        }

    }


}

void Input::cursorPosCallback(GLFWwindow* pWindow, double xpos, double ypos) {
    if (!cam1.rotating) return;

    if (cam1.firstMouse) {
        cam1.lastX = xpos; cam1.lastY = ypos;
        cam1.firstMouse = false;
        return;
    }

    double xoffset = xpos - cam1.lastX;
    double yoffset = cam1.lastY - ypos; // invert: screen Y grows downward
    cam1.lastX = xpos; cam1.lastY = ypos;

    cam1.yaw += static_cast<float>(xoffset) * cam1.mouseSens;
    cam1.pitch += static_cast<float>(yoffset) * cam1.mouseSens;

    // clamp pitch to avoid gimbal flip
    if (cam1.pitch > 89.0f)  cam1.pitch = 89.0f;
    if (cam1.pitch < -89.0f) cam1.pitch = -89.0f;

    cam1.updateFrontYawPitch();
}

void Input::mouseButtonCallback(GLFWwindow* pWindow, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) { // choose your button
        if (action == GLFW_PRESS) {
            cam1.rotating = true;
            cam1.firstMouse = true; // reset on engage
            glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            if (glfwRawMouseMotionSupported()) {
                glfwSetInputMode(pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }
        }
        else if (action == GLFW_RELEASE) {
            cam1.rotating = false;
            cam1.firstMouse = false;
            glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            if (glfwRawMouseMotionSupported()) {
                glfwSetInputMode(pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
            }
        }
    }
}