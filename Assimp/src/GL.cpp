#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "GL.h"
#include "camera.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <cmath>


GLFWwindow* GLSetup::window;
unsigned int GLSetup::shaderProgram;
extern Camera cam1;

void Mesh::setupMesh() {
    if (vertices.empty()) {
        std::cerr << "Vertices is Empty\n";
        return;
    }

    glCreateVertexArrays(1, &VAO);
    glCreateBuffers(1, &VBO);
    glNamedBufferData(VBO, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(Vertex));

    //position attribute
    glEnableVertexArrayAttrib(VAO, 0);
    glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, (GLuint)offsetof(Vertex, pos));
    glVertexArrayAttribBinding(VAO, 0, 0);

    //normal attribute
    glEnableVertexArrayAttrib(VAO, 1);
    glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, (GLuint)offsetof(Vertex, normal));
    glVertexArrayAttribBinding(VAO, 1, 0);

    if (!indices.empty()) {
        glCreateBuffers(1, &EBO);
        glNamedBufferData(EBO, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        glVertexArrayElementBuffer(VAO, EBO);
    }
}

void Mesh::draw() const {
    if (mat.twoSided) glDisable(GL_CULL_FACE);
    glUseProgram(GLSetup::shaderProgram);
    glBindVertexArray(VAO);
    glUniform3fv(glGetUniformLocation(GLSetup::shaderProgram, "uKa"), 1, &mat.Ka.x);
    glUniform3fv(glGetUniformLocation(GLSetup::shaderProgram, "uKd"), 1, &mat.Kd.x);
    glUniform3fv(glGetUniformLocation(GLSetup::shaderProgram, "uKs"), 1, &mat.Ks.x);
    glUniform1f(glGetUniformLocation(GLSetup::shaderProgram, "uShininess"), mat.shininess);
    glUniform1f(glGetUniformLocation(GLSetup::shaderProgram, "uOpacity"), mat.opacity);
    glUniform1i(glGetUniformLocation(GLSetup::shaderProgram, "matToggle"), GLSetup::matToggle);

    if (!indices.empty()) {
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr);
    }
    else {
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());
    }
    glBindVertexArray(0);
    if (mat.twoSided) glEnable(GL_CULL_FACE);
}

Model Model::load(const char* path) {

    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("Invalid file path");
    }

    unsigned flags =
        aiProcess_Triangulate | aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality |
        aiProcess_PreTransformVertices | aiProcess_GlobalScale |
        aiProcess_OptimizeMeshes | aiProcess_CalcTangentSpace;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, flags);

    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode || scene->mNumMeshes == 0) {
        std::cerr << "ASSIMP error: " << importer.GetErrorString() << "\n";
        throw std::runtime_error("ASSIMP error");
    }

    Model model;
    model.meshes.reserve(scene->mNumMeshes);

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh* m = scene->mMeshes[i];

        std::vector<Vertex> verts; 
        verts.reserve(m->mNumVertices);
        for (unsigned int j = 0; j < m->mNumVertices; ++j) {
            
            Vertex vert{};

            vert.pos = { m->mVertices[j].x, m->mVertices[j].y, m->mVertices[j].z };
            if (m->HasNormals()) {
                vert.normal = { m->mNormals[j].x, m->mNormals[j].y, m->mNormals[j].z };
            }
            else {
                vert.normal = { 0, 1, 0 };
            }
            verts.push_back(vert);
        }

        std::vector<unsigned> idx; 
        idx.reserve(m->mNumFaces * 3);
        for (unsigned f = 0; f < m->mNumFaces; ++f) {
            const aiFace& face = m->mFaces[f];
            for (unsigned k = 0; k < face.mNumIndices; ++k)
            {
                idx.push_back(face.mIndices[k]);
            }
        }

        Material mat;

        if (scene->HasMaterials()) {
            const aiMaterial* A = scene->mMaterials[m->mMaterialIndex];
            aiColor4D c;
            float f;
            int iVal;
            if (AI_SUCCESS == aiGetMaterialColor(A, AI_MATKEY_COLOR_AMBIENT, &c)) mat.Ka = { c.r,c.g,c.b };
            if (AI_SUCCESS == aiGetMaterialColor(A, AI_MATKEY_COLOR_DIFFUSE, &c)) {
                mat.Kd = { c.r,c.g,c.b };
            } 
            if (AI_SUCCESS == aiGetMaterialColor(A, AI_MATKEY_COLOR_SPECULAR, &c)) mat.Ks = { c.r,c.g,c.b };
            if (AI_SUCCESS == aiGetMaterialFloat(A, AI_MATKEY_SHININESS, &f))   mat.shininess = (f > 0 ? f : 16.0f);
            if (AI_SUCCESS == aiGetMaterialFloat(A, AI_MATKEY_OPACITY, &f))   mat.opacity = glm::clamp(f, 0.f, 1.f);
            if (AI_SUCCESS == aiGetMaterialInteger(A, AI_MATKEY_TWOSIDED, &iVal)) mat.twoSided = (iVal != 0);
        }

        Mesh mesh(verts, idx);
        mesh.mat = mat;
        model.meshes.push_back(mesh);

    }

    return model;
}

void Model::draw() const {
    for (auto& mesh : meshes) {
        mesh.draw();
    }
}


void GLSetup::init() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    //create window object
    window = glfwCreateWindow(800, 600, "Assimp", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);

    //init GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    // Set GLFW callback functions
    glfwSetKeyCallback(window, Input::keyCallback);
    glfwSetMouseButtonCallback(window, Input::mouseButtonCallback);
    glfwSetCursorPosCallback(window, Input::cursorPosCallback);

    //viewport
    //first 2 params indicate lower left corner of window
    glViewport(0, 0, 800, 600);

    //resize viewport
    //callback function - only gets called when an event happens
    glfwSetFramebufferSizeCallback(window, GLSetup::framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST);


    //cmd comments
    std::cout << "WASD to move camera around\nSPACE to fly up\nCRTL to fly down\nENTER to toggle material\n";
    std::cout << "Hold Right Mouse Button to look around\n";
}




void GLSetup::setupShader() {
    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    std::string vertexShaderString = loadShaderSource("Shaders/vertex_shader.vert");
    const char* vertexShaderSource = vertexShaderString.c_str();
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for vertex shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // fragment shader
    std::string fragmentShaderString = loadShaderSource("Shaders/fragment_shader.frag");
    const char* fragmentShaderSource = fragmentShaderString.c_str();
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for vertex shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    //link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void GLSetup::setMat4(GLuint prog, const char* name, const glm::mat4& M) {
    glUniformMatrix4fv(glGetUniformLocation(prog, name), 1, GL_FALSE, glm::value_ptr(M));
}
void GLSetup::setVec3(GLuint prog, const char* name, const glm::vec3& v) {
    glUniform3fv(glGetUniformLocation(prog, name), 1, glm::value_ptr(v));
}

std::string GLSetup::loadShaderSource(const char* filepath) {

    if (!std::filesystem::exists(filepath)) {
        return "Invalid File Path\n";
    }

    std::ifstream file(filepath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}