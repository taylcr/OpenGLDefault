#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <unordered_set>

using namespace std;

// 1-  Define the Vertex Shader and Fragment Shader:
//     Create a simple vertex and fragment shader to render the shapes.

        //GLSL (OpenGL Shading Language)
const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 transform;
void main()
{
    gl_Position = transform * vec4(aPos, 1.0);
}
)glsl";
// rain bow gradiant color
const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;
uniform float time;
void main()
{
    float r = 0.5 * sin(time + 0.0) + 0.5;
    float g = 0.5 * sin(time + 2.0) + 0.5;
    float b = 0.5 * sin(time + 4.0) + 0.5;
    FragColor = vec4(r, g, b, 1.0);
}
)glsl";

// Step 3: Applying Transformations
        // 1. Create Transformation Matrices:
        // Use GLM (OpenGL Mathematics) library to create transformation matrices.
        // 2. Add Uniforms for Transformation Matrices in the Shader:
        // Update the vertex shader to accept a transformation matrix.
        // 3. Apply Transformations:

const unordered_set<int> validKeys = {
    GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D,
    GLFW_KEY_Q, GLFW_KEY_E, GLFW_KEY_R, GLFW_KEY_F, GLFW_KEY_ESCAPE
};

bool processInput(GLFWwindow* window, glm::mat4& transform) {
    bool validInput = false;
    float translationSpeed = 0.01f; // Speed of translation
    float rotationSpeed = glm::radians(30.0f); // 30 degrees rotation
    float scaleSpeed = 0.4f; // Scaling factor

    // Translation 
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

    // Rotation 
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        transform = glm::rotate(transform, rotationSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
        validInput = true;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        transform = glm::rotate(transform, -rotationSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
        validInput = true;
    }

    // Scaling 
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        transform = glm::scale(transform, glm::vec3(scaleSpeed, scaleSpeed, scaleSpeed));
        validInput = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        transform = glm::scale(transform, glm::vec3(1/scaleSpeed, 1/scaleSpeed, 1/scaleSpeed));
        validInput = true;
    }

    // Close the window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
        validInput = true;
    }

    // Check for invalid keys
    if (!validInput) {
        for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key) {  //iterates over all possible key values from GLFW_KEY_SPACE to GLFW_KEY_LAST (GLFW)
            if (glfwGetKey(window, key) == GLFW_PRESS && validKeys.find(key) == validKeys.end()) {
                cout << "Invalid key pressed!" << endl;
                break;
            }
        }
    }

    return validInput;
}


int main() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

// window creation
    GLFWwindow* window = glfwCreateWindow(600, 600, "Assignment 2", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to initial window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewInit(); // OpenGL Extension Wrangler Library


//  2- Compile and Link the Shaders:
//     Compile the vertex and fragment shaders and link them into a shader program.
  
     // vertext shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); //vertexShaderSource
    glCompileShader(vertexShader);

    //fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); //fragmentShaderSource
    glCompileShader(fragmentShader);

    // Shader program
    unsigned int shaderProgram = glCreateProgram(); // attaching vertex and fragment shaders to the program
    
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

// 3. Define the Vertex Data for a Triangle:

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,  // Bottom left
         0.5f, -0.5f, 0.0f,  // Bottom right
         0.0f,  0.5f, 0.0f   // Top
    };

//4. Set Up the Vertex Array Object (VAO), Vertex Buffer Object (VBO), and Element Buffer Object (EBO):
        //Low Vertex Count so no EBO

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO); //Generate and Bind VAO
    glGenBuffers(1, &VBO);   // able to Stores one triangle object
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(0);

    glm::mat4 transform = glm::mat4(1.0f);
    unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    unsigned int timeLoc = glGetUniformLocation(shaderProgram, "time"); // to create rain bow effect dynamically

// 5. Render the Shapes
// Render the Shapes
    while (!glfwWindowShouldClose(window)) {
         
         //give window state and transform
        processInput(window, transform);

        glClearColor(0.529f, 0.808f, 0.980f, 1.0f);  // Sky blue color
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        //// Sets the value of the  variable transform in the shader program
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
        glUniform1f(timeLoc, glfwGetTime());
        glBindVertexArray(VAO);
        //Draws the triangles based on the currently bound VAO and its associated VBO
        glDrawArrays(GL_TRIANGLES, 0, 3);

       

        //Swaps the front and back buffers.
        glfwSwapBuffers(window);
        //Processes events
        glfwPollEvents();
    }


    glfwTerminate();
    return 0;
}
