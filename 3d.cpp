#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Werror"


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <unordered_set>
#include <vector>

using namespace std;

// Vertex Shader
const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 transform;
void main()
{
    gl_Position = transform * vec4(aPos, 1.0);
}
)glsl";

// Fragment Shader
const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;
uniform float time;
void main()
{
    float r = 0.5 * sin(time + 1.5) + 0.6;
    float g = 0.5 * sin(time + 2.0) + 1.5;
    float b = 0.5 * sin(time + 4.0) + 0.4;
    FragColor = vec4(r, g, b, 1.0);
}
)glsl";

// Key mappings
const unordered_set<int> validKeys = {
    GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D,
    GLFW_KEY_Q, GLFW_KEY_E, GLFW_KEY_R, GLFW_KEY_F, GLFW_KEY_T, GLFW_KEY_ESCAPE
};

void printUI() {
    cout << "\n------------- Control Commands --------------\n\n";
    cout << "W: Move up\n";
    cout << "S: Move down\n";
    cout << "A: Move left\n";
    cout << "D: Move right\n";
    cout << "Q: Rotate counter-clockwise\n";
    cout << "E: Rotate clockwise\n";
    cout << "R: Scale up\n";
    cout << "F: Scale down\n";
    cout << "T: Reset position\n";
    cout << "ESC: Close the window\n";
    cout << "\n\n";
    cout << "\n---------------------------------------------\n\n";
}

bool processInput(GLFWwindow* window, glm::mat4& transform) {
    bool validInput = false;
    
    float translationSpeed = 0.01f;
    float rotationSpeed = glm::radians(30.0f);
    float scaleSpeed = 0.95f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        transform = glm::translate(transform, glm::vec3(0.0f, translationSpeed, 0.0f));
        validInput = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        transform = glm::translate(transform, glm::vec3(0.0f, -translationSpeed, 0.0f));
        validInput = true;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        transform = glm::translate(transform, glm::vec3(-translationSpeed, 0.0f, 0.0f));
        validInput = true;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        transform = glm::translate(transform, glm::vec3(translationSpeed, 0.0f, 0.0f));
        validInput = true;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        transform = glm::rotate(transform, rotationSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
        validInput = true;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        transform = glm::rotate(transform, -rotationSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
        validInput = true;
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        transform = glm::scale(transform, glm::vec3(scaleSpeed, scaleSpeed, scaleSpeed));
        validInput = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        transform = glm::scale(transform, glm::vec3(1/scaleSpeed, 1/scaleSpeed, 1/scaleSpeed));
        validInput = true;
    }

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        transform = glm::mat4(1.0f);
        validInput = true;
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
        validInput = true;
    }

    if (!validInput) {
        for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key) {
            if (glfwGetKey(window, key) == GLFW_PRESS && validKeys.find(key) == validKeys.end()) {
                cout << "Invalid key pressed!" << endl;
                break;
            }
        }
    }

    return validInput;
}

const aiScene* loadModel(std::string path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return nullptr;
    }
    return scene;
}

std::vector<float> vertices;
void processMesh(aiMesh* mesh, const aiScene* scene) {
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);
    }
}

void processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene);
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Model", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to initialize window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewInit();

    // Compile vertex and fragment shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Shader program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Load and process the 3D model
    const aiScene* scene = loadModel("bugatti.obj");
    if (!scene) {
        return -1;
    }
    processNode(scene->mRootNode, scene);

    // Set up VAO and VBO for the model
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glm::mat4 transform = glm::mat4(1.0f);
    unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    unsigned int timeLoc = glGetUniformLocation(shaderProgram, "time");

    while (!glfwWindowShouldClose(window)) {
        processInput(window, transform);

        glClearColor(0.42f, 0.74f, 0.92f, 0.7f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
        glUniform1f(timeLoc, glfwGetTime());

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}


#pragma GCC diagnostic pop
