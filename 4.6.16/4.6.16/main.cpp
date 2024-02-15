#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<vector>

#include"ShaderClass.h"

using namespace std;

const GLuint WIDTH = 1000, HEIGHT = 1000;
const GLfloat cameraSpeed = 0.001f;
glm::vec3 cubePosition = glm::vec3(0.0f, 0.5f, 0.0f); // Initial position of the cube


// Camera settings
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
GLfloat yaw = -90.0f;
GLfloat pitch = 0.0f;
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool firstMouse = true;
bool keys[1024];

// Mouse movement callback function
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    GLfloat sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// Keyboard input callback function
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    GLfloat cameraSpeed = 0.05f;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

// Function to calculate the inverse of a 4x4 matrix
glm::mat4 inverseMatrix(const glm::mat4& mat) {
    return glm::inverse(mat);
}

// Function to perform matrix multiplication
glm::vec4 multiplyMatrixVector(const glm::mat4& mat, const glm::vec4& vec) {
    return mat * vec;
}

// Function to perform interpolation and generate the polynomial coefficients
std::vector<float> interpolatePoints(const std::vector<glm::vec2>& points) {
    glm::mat4 A(1, 1, 1, 1,
        8, 4, 2, 1,
        216, 36, 6, 1,
        512, 64, 8, 1);

    glm::vec4 b(1, 8, 3, 1);

    glm::mat4 A_inv = inverseMatrix(A);
    glm::vec4 x = multiplyMatrixVector(A_inv, b);

    std::vector<float> coefficients;
    coefficients.push_back(x[0]);
    coefficients.push_back(x[1]);
    coefficients.push_back(x[2]);
    coefficients.push_back(x[3]);

    return coefficients;
}

// Function to evaluate the polynomial at a given x value
float evaluatePolynomial(const std::vector<float>& coefficients, float x) {
    float result = 0.0f;
    float x_pow = 1.0f;
    for (int i = 0; i < coefficients.size(); ++i) {
        result += coefficients[i] * x_pow;
        x_pow *= x;
    }
    return result;
}

// OpenGL rendering function
void render(const std::vector<glm::vec2>& points, const std::vector<float>& coefficients)
{
    // Vertex data for points
    std::vector<float> pointVertices;
    for (const auto& point : points) {
        pointVertices.push_back(point.x);
        pointVertices.push_back(point.y);
    }

    // Vertex data for graph
    std::vector<float> graphVertices;
    for (float x = 1.0f; x <= 8.0f; x += 0.1f) {
        float y = evaluatePolynomial(coefficients, x);
        graphVertices.push_back(x);
        graphVertices.push_back(y);
    }

    // Create and bind vertex buffer for points
    GLuint pointVBO;
    glGenBuffers(1, &pointVBO);
    glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * pointVertices.size(), pointVertices.data(), GL_STATIC_DRAW);

    // Create and bind vertex buffer for graph
    GLuint graphVBO;
    glGenBuffers(1, &graphVBO);
    glBindBuffer(GL_ARRAY_BUFFER, graphVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * graphVertices.size(), graphVertices.data(), GL_STATIC_DRAW);

    // Set up vertex attributes for points
    GLuint pointVAO;
    glGenVertexArrays(1, &pointVAO);
    glBindVertexArray(pointVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Set up vertex attributes for graph
    GLuint graphVAO;
    glGenVertexArrays(1, &graphVAO);
    glBindVertexArray(graphVAO);
    glBindBuffer(GL_ARRAY_BUFFER, graphVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

}



int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Cube with Camera", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);

    //Generates Shader object using shaders defualt.vert and default.frag
    Shader shaderProgram("default.vert", "default.frag");

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);



    // Define vertices of the cube
    GLfloat vertices[] = {
        // Positions
        -0.5f, -0.5f, -0.5f,        1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,         1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f,          1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f,          1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f,         1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,        1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f,         1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f,          1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f,           1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f,           1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f,          1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f,         1.0f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f,          1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f,         1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,        1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,        1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f,         1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f,          1.0f, 0.0f, 0.0f,

        0.5f, 0.5f, 0.5f,           1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f,          1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,         1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,         1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f,          1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f,           1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,        1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,         1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f,          1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f,          1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f,         1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,        1.0f, 0.0f, 0.0f,

        -0.5f, 0.5f, -0.5f,         1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f,          1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f,           1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f,           1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f,          1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f,         1.0f, 0.0f, 0.0f,
    };

    GLuint Indices[] =
    {
    0, 1, 2,
    2, 3, 0,
    };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

    // Game loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        GLfloat deltaTime = glfwGetTime();
        glfwSetTime(0.0f);

        // Handle keyboard input for camera movement
        if (keys[GLFW_KEY_W])
            cameraPos += cameraSpeed * cameraFront;
        if (keys[GLFW_KEY_S])
            cameraPos -= cameraSpeed * cameraFront;
        if (keys[GLFW_KEY_A])
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (keys[GLFW_KEY_D])
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;


        // Clear the color and depth buffers
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use shader program
        shaderProgram.Activate();

        // View matrix
        glm::mat4 view;
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // Pass transformation matrices to shader
        GLint modelLoc = glGetUniformLocation(shaderProgram.ID, "model");
        GLint viewLoc = glGetUniformLocation(shaderProgram.ID, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram.ID, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        //// Draw the cube
        //glBindVertexArray(VAO);
        //glDrawArrays(GL_TRIANGLES, 0, 36);
        //glBindVertexArray(0);


        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    
    glfwTerminate();

    return 0;
}




