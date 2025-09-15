#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION

#include "GL.h"
#include "camera.h"
#include "stb_image.h"


#include <iostream>

Camera cam1;

int main() {

    GLSetup::init();

    GLSetup::setupShader();

    Model ak47;
    Model dragon;

    try {
        ak47 = Model::load("assets\\models\\AK47.fbx");
        dragon = Model::load("assets\\models\\Dragon 2.5_fbx.fbx");
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        glfwSetWindowShouldClose(GLSetup::window, true);
    }

    GLSetup::lastTime = static_cast<float>(glfwGetTime());
    GLSetup::lastTime2 = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(GLSetup::window)) {

        glfwPollEvents();
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Camera::CameraUpdate();

        //ak47.update();
        //ak47.draw();
        dragon.update();
        dragon.draw();

        glfwSwapBuffers(GLSetup::window);
    }

    glfwDestroyWindow(GLSetup::window);
    glfwTerminate();
    return 0;
}
