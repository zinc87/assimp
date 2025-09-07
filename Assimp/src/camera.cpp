#include "camera.h"

extern Camera cam1;

void Camera::updateFrontYawPitch() {
    const float cy = glm::cos(glm::radians(yaw));
    const float sy = glm::sin(glm::radians(yaw));
    const float cp = glm::cos(glm::radians(pitch));
    const float sp = glm::sin(glm::radians(pitch));
    camFront = glm::normalize(glm::vec3(cy * cp, sp, sy * cp));
}

void Camera::CameraUpdate() {

    float now = static_cast<float>(glfwGetTime());
    float dt = now - GLSetup::lastTime;
    GLSetup::lastTime = now;

    // --- per-frame WASD movement ---
    float v = cam1.moveSpeed * dt;
    glm::vec3 camRight = glm::normalize(glm::cross(cam1.camFront, cam1.camUp));

    if (glfwGetKey(GLSetup::window, GLFW_KEY_W) == GLFW_PRESS) cam1.camPos += cam1.camFront * v;
    if (glfwGetKey(GLSetup::window, GLFW_KEY_S) == GLFW_PRESS) cam1.camPos -= cam1.camFront * v;
    if (glfwGetKey(GLSetup::window, GLFW_KEY_A) == GLFW_PRESS) cam1.camPos -= camRight * v;
    if (glfwGetKey(GLSetup::window, GLFW_KEY_D) == GLFW_PRESS) cam1.camPos += camRight * v;
    if (glfwGetKey(GLSetup::window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) cam1.camPos -= cam1.camUp * v; // down
    if (glfwGetKey(GLSetup::window, GLFW_KEY_SPACE) == GLFW_PRESS) cam1.camPos += cam1.camUp * v; // up

    // simple camera
    glm::mat4 V = glm::lookAt(cam1.camPos, cam1.camPos + cam1.camFront, cam1.camUp);
    glm::mat4 P = glm::perspective(glm::radians(60.0f), 800.f / 600.f, 0.1f, 100.f);

    GLSetup::setMat4(GLSetup::shaderProgram, "uView", V);
    GLSetup::setMat4(GLSetup::shaderProgram, "uProj", P);
    GLSetup::setVec3(GLSetup::shaderProgram, "uCamPos", cam1.camPos);
}
