#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>

#include "Input.h"

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

struct Material {
	glm::vec3 Ka{ 0.2f }; //ambient
	glm::vec3 Kd{ 0.7f }; //diffuse
	glm::vec3 Ks{ 0.1f }; //specular
	float shininess = 10.0f;
	float opacity = 1.0f;
	bool twoSided = false;
};


struct GLSetup {

	inline static int width = 1000;
	inline static int height = 800;
	inline static float lastTime;
	inline static float lastTime2;
	inline static bool matToggle = false;

	static void init();
	static void setupShader();

	static void setMat4(GLuint prog, const char* name, const glm::mat4& M);
	static void setMat3(GLuint prog, const char* name, const glm::mat3& M);
	static void setVec3(GLuint prog, const char* name, const glm::vec3& v);

	static GLFWwindow* window;
	static unsigned int shaderProgram;

private:

	static std::string loadShaderSource(const char* filepath);
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	}

};

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	Material mat;

	GLuint normalTex;
	bool hasNormalMap = false;

	Mesh() = default;
	Mesh(const std::vector<Vertex>& v, const std::vector<unsigned int>& i) :
		vertices{ v }, indices{ i }
	{
		setupMesh();
	}

	void draw() const;

private:
	GLuint VAO = 0, VBO = 0, EBO = 0;
	void setupMesh();
};


class Model {
public:
	glm::vec3 position;
	float rotation;
	glm::vec3 scale;

	std::vector<Mesh> meshes;
	static Model load(const char* modelPath);
	void update();
	void draw() const;
};

