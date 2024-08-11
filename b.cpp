#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "tiny_obj_loader.h"
#include <iostream>
#include <vector>

using namespace std;

// Camera variables
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;  // Time between current frame and last frame
float lastFrame = 0.0f;  // Time of last frame

float yaw = -90.0f;  // Yaw is initialized to -90.0 degrees to align the camera front with the negative Z axis
float pitch = 0.0f;
float lastX = 400.0f, lastY = 300.0f;
bool firstMouse = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05f;  // Adjust sensitivity (lower value means less sensitivity)
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Make sure that when pitch is out of bounds, the screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void processInput(GLFWwindow* window, glm::mat4 &model) {
    float cameraSpeed = 3.0f * deltaTime;  // Adjust speed here
    float rotationSpeed = glm::radians(30.0f) * deltaTime;  // Rotation speed
    float scaleSpeed = 1.0f + 0.5f * deltaTime;  // Scaling factor

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        // Rotate left (Q)
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            model = glm::rotate(model, rotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));

        // Rotate right (E)
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            model = glm::rotate(model, -rotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));

        // Translate up (W)
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            model = glm::translate(model, glm::vec3(0.0f, cameraSpeed, 0.0f));

        // Translate down (S)
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            model = glm::translate(model, glm::vec3(0.0f, -cameraSpeed, 0.0f));

        // Translate left (A)
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            model = glm::translate(model, glm::vec3(-cameraSpeed, 0.0f, 0.0f));

        // Translate right (D)
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            model = glm::translate(model, glm::vec3(cameraSpeed, 0.0f, 0.0f));

        // Scale up (R)
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
            model = glm::scale(model, glm::vec3(scaleSpeed, scaleSpeed, scaleSpeed));

        // Scale down (F)
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
            model = glm::scale(model, glm::vec3(1.0f / scaleSpeed, 1.0f / scaleSpeed, 1.0f / scaleSpeed));

        // Reset (T)
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
            model = glm::mat4(1.0f);  // Reset to identity matrix
    }

    // Camera controls (for normal movement)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    // Move up
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;

    // Move down
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;

    // Close window on ESC key press
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Vertex Shader Source Code
const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)glsl";

// Fragment Shader Source Code
const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main()
{
    // Ambient
    vec3 ambient = light.ambient * material.ambient;
    
    // Diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);
    
    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);  
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
)glsl";

// Function to load shaders
unsigned int loadShader(const char* source, GLenum type) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << endl;
    }

    return shader;
}

void loadModel(const std::string& path, std::vector<float>& vertices, std::vector<float>& normals) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        throw std::runtime_error(warn + err);
    }

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            vertices.push_back(attrib.vertices[3 * index.vertex_index + 0]);
            vertices.push_back(attrib.vertices[3 * index.vertex_index + 1]);
            vertices.push_back(attrib.vertices[3 * index.vertex_index + 2]);

            normals.push_back(attrib.normals[3 * index.normal_index + 0]);
            normals.push_back(attrib.normals[3 * index.normal_index + 1]);
            normals.push_back(attrib.normals[3 * index.normal_index + 2]);
        }
    }
}

int main() {
    // Initialize GLFW and OpenGL
    if (!glfwInit()) {
        cout << "Failed to initialize GLFW" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Display Bugatti", NULL, NULL);
    if (!window) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        cout << "Failed to initialize GLEW" << endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load shaders and create shader program
    unsigned int vertexShader = loadShader(vertexShaderSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = loadShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Load the Bugatti model
    std::vector<float> vertices, normals;
    try {
        loadModel("LibertyStatue.obj", vertices, normals);  // Provide the correct path to your .obj file
    } catch (const std::exception& e) {
        cout << "Error loading model: " << e.what() << endl;
        return -1;
    }

    unsigned int VBO, VAO, NBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &NBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Initialize model matrix
    glm::mat4 model = glm::mat4(1.0f);

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate deltaTime
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        processInput(window, model);

        // Clear the screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Set light and material properties
        glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "light.position"), lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "light.ambient"), 0.2f, 0.2f, 0.2f);
        glUniform3f(glGetUniformLocation(shaderProgram, "light.diffuse"), 0.5f, 0.5f, 0.5f);
        glUniform3f(glGetUniformLocation(shaderProgram, "light.specular"), 1.0f, 1.0f, 1.0f);

        glUniform3f(glGetUniformLocation(shaderProgram, "material.ambient"), 1.0f, 0.5f, 0.31f);
        glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuse"), 1.0f, 0.5f, 0.31f);
        glUniform3f(glGetUniformLocation(shaderProgram, "material.specular"), 0.5f, 0.5f, 0.5f); // Specular highlight
        glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), 32.0f);

        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);

        // Adjust the view and projection matrices
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);

        // Pass these matrices to the shader
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Bind and draw the model
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);
        glBindVertexArray(0);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &NBO);

    glfwTerminate();
    return 0;
}
