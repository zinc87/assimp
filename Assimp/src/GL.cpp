#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include "GL.h"
#include "camera.h"
#include "stb_image.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <cmath>
#include <filesystem>
#include <string.h>

using std::filesystem::path;

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

    //position attribute (location=0)
    glEnableVertexArrayAttrib(VAO, 0);
    glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, (GLuint)offsetof(Vertex, pos));
    glVertexArrayAttribBinding(VAO, 0, 0);

    //normal attribute (location=1)
    glEnableVertexArrayAttrib(VAO, 1);
    glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, (GLuint)offsetof(Vertex, normal));
    glVertexArrayAttribBinding(VAO, 1, 0);

    //uv attribute (location=2)
    glEnableVertexArrayAttrib(VAO, 2);
    glVertexArrayAttribFormat(VAO, 2, 2, GL_FLOAT, GL_FALSE, (GLuint)offsetof(Vertex, uv));
    glVertexArrayAttribBinding(VAO, 2, 0);

    // tangent attribute (location=3)
    glEnableVertexArrayAttrib(VAO, 3);
    glVertexArrayAttribFormat(VAO, 3, 3, GL_FLOAT, GL_FALSE, (GLuint)offsetof(Vertex, tangent));
    glVertexArrayAttribBinding(VAO, 3, 0);

    // bitangent attribute (location=4)
    glEnableVertexArrayAttrib(VAO, 4);
    glVertexArrayAttribFormat(VAO, 4, 3, GL_FLOAT, GL_FALSE, (GLuint)offsetof(Vertex, bitangent));
    glVertexArrayAttribBinding(VAO, 4, 0);

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

    //bind normal map on texture unit 0
    GLint locHasNM = glGetUniformLocation(GLSetup::shaderProgram, "uHasNormalMap");
    GLint locNm = glGetUniformLocation(GLSetup::shaderProgram, "uNormalMap");
    if (hasNormalMap && normalTex) {
        glBindTextureUnit(0, normalTex);
        glUniform1i(locNm, 0);
        glUniform1i(locHasNM, 1);
    }
    else {
        glUniform1i(locHasNM, 0);
    }

    if (!indices.empty()) {
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr);
    }
    else {
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());
    }
    glBindVertexArray(0);
    if (mat.twoSided) glEnable(GL_CULL_FACE);
}

Model Model::load(const char* modelPath) {

    if (!std::filesystem::exists(modelPath)) {
        throw std::runtime_error("Invalid file path");
    }

    path modelDir = path(modelPath).parent_path();

    unsigned flags =
        aiProcess_Triangulate | aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality |
        aiProcess_PreTransformVertices | aiProcess_GlobalScale |
        aiProcess_OptimizeMeshes | aiProcess_CalcTangentSpace;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(modelPath, flags);

    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode || scene->mNumMeshes == 0) {
        std::cerr << "ASSIMP error: " << importer.GetErrorString() << "\n";
        throw std::runtime_error("ASSIMP error");
    }

    Model model;

    GLuint normalTexID = 0;
    bool hasNormal = false;

    model.meshes.reserve(scene->mNumMeshes);

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh* m = scene->mMeshes[i];


        //VERTEX
        std::vector<Vertex> verts; 
        verts.reserve(m->mNumVertices);
        for (unsigned int j = 0; j < m->mNumVertices; ++j) {
            
            Vertex vert{};
            
            //postition
            vert.pos = { m->mVertices[j].x, m->mVertices[j].y, m->mVertices[j].z };
            
            //normal
            if (m->HasNormals()) {
                vert.normal = { m->mNormals[j].x, m->mNormals[j].y, m->mNormals[j].z };
            }
            else {
                vert.normal = { 0, 1, 0 };
            }

            //UV
            if (m->HasTextureCoords(0)) {
                vert.uv = { m->mTextureCoords[0][j].x, m->mTextureCoords[0][j].y };
            }
            else {
                vert.uv = { 0.f,0.f };
            }

            //tangent/Bitangent
            if (m->HasTangentsAndBitangents()) {
                vert.tangent = { m->mTangents[j].x,m->mTangents[j].y ,m->mTangents[j].z };
                vert.bitangent = { m->mBitangents[j].x, m->mBitangents[j].y,m->mBitangents[j].z };
            }
            else {
                vert.tangent = { 1,0,0 };
                vert.bitangent = { 0,1,0 };
            }
            verts.push_back(vert);
        }//END VERTEX

        //INDEX
        std::vector<unsigned> idx; 
        idx.reserve(m->mNumFaces * 3);
        for (unsigned f = 0; f < m->mNumFaces; ++f) {
            const aiFace& face = m->mFaces[f];
            for (unsigned k = 0; k < face.mNumIndices; ++k)
            {
                idx.push_back(face.mIndices[k]);
            }
        }//END INDEX

        //MATERIAL
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


            //NORMAL MAPPING - no embedded textures
            auto loadNormalMap = [](const char* fullpath, bool genMip = true) -> GLuint {

                int w, h, n;
                stbi_set_flip_vertically_on_load(true);
                unsigned char* textureData = stbi_load(fullpath, &w, &h, &n, 4);
                if (!textureData) {
                    std::cerr << "stb_image error: " << stbi_failure_reason()
                        << "\nTried: " << fullpath << "\n";
                    return 0;
                }

                GLuint tex;
                glCreateTextures(GL_TEXTURE_2D, 1, &tex);
                glTextureStorage2D(tex, std::max(1, (int)std::floor(std::log2(std::max(w, h)))) + 1, GL_RGBA8, w, h);
                glTextureSubImage2D(tex, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
                if (genMip) glGenerateTextureMipmap(tex);
                glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_REPEAT);
                stbi_image_free(textureData);
                return tex;

            };


            aiString texPath;

            if (A->GetTextureCount(aiTextureType_NORMALS) > 0 &&
                A->GetTexture(aiTextureType_NORMALS, 0, &texPath) == AI_SUCCESS) {
                std::filesystem::path rel(texPath.C_Str());
                std::filesystem::path full = modelDir / rel;
               
                normalTexID = loadNormalMap(full.string().c_str());
                hasNormal = (normalTexID != 0);
            }
            else if (A->GetTextureCount(aiTextureType_HEIGHT) > 0 &&
                A->GetTexture(aiTextureType_HEIGHT, 0, &texPath) == AI_SUCCESS) {

                std::filesystem::path rel(texPath.C_Str());
                std::filesystem::path full = modelDir / rel;

                normalTexID = loadNormalMap(full.string().c_str());
                hasNormal = (normalTexID != 0);
            }
        }//END MATERIAL




        Mesh mesh(verts, idx);
        mesh.mat = mat;
        mesh.normalTex = normalTexID;
        mesh.hasNormalMap = hasNormal;
        model.meshes.push_back(mesh);

    }

    model.position = glm::vec3(0.f);
    model.rotation = 0.f;
    model.scale = glm::vec3(1.f);

    return model;
}

void Model::update(){
    float now = static_cast<float>(glfwGetTime());
    float dt = now - GLSetup::lastTime2;
    GLSetup::lastTime2 = now;
    
    float moveSpeed = 10.f;
    float v = moveSpeed * dt;

    if (glfwGetKey(GLSetup::window, GLFW_KEY_RIGHT) == GLFW_PRESS) position.z += 1.f * v;
    if (glfwGetKey(GLSetup::window, GLFW_KEY_LEFT) == GLFW_PRESS) position.z -= 1.f * v;
    if (glfwGetKey(GLSetup::window, GLFW_KEY_UP) == GLFW_PRESS) position.y += 1.f * v;
    if (glfwGetKey(GLSetup::window, GLFW_KEY_DOWN) == GLFW_PRESS) position.y -= 1.f * v;
}

void Model::draw() const {

    //transform pos,rotation,scale from model to world
    glm::mat4 M = glm::translate(glm::mat4(1.f), position)
        * glm::rotate(glm::mat4(1.f), rotation, glm::vec3(0, 1, 0))
        * glm::scale(glm::mat4(1.f), scale);
    //transform normals from model to world
    glm::mat3 normalMat = glm::transpose(glm::inverse(M));
    GLSetup::setMat4(GLSetup::shaderProgram, "uModel", M);
    GLSetup::setMat3(GLSetup::shaderProgram, "uNormalMat", normalMat);

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
    window = glfwCreateWindow(GLSetup::width, GLSetup::height, "Assimp", NULL, NULL);
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
    glViewport(0, 0, GLSetup::width, GLSetup::height);

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
void GLSetup::setMat3(GLuint prog, const char* name, const glm::mat3& M) {
    glUniformMatrix3fv(glGetUniformLocation(prog, name), 1, GL_FALSE, glm::value_ptr(M));
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