#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<vector>

#include"ShaderClass.h"

using namespace std;

GLFWwindow* window;

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

//vector<float> A_matrise = { 1, 1, 1, 1, 8, 4, 2, 1, 216, 36, 6, 1, 512, 64, 8, 1 };
//vector<float> A_vektor = { 1, 8, 3, 1 };

vector<glm::vec2> controlPoints = { glm::vec2(1, 1), glm::vec2(2, 8), glm::vec2(6, 3), glm::vec2(8, 1) };

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
//void savePointsToFile(const std::vector<glm::vec2>& points, const std::string& filename);
//glm::vec4 inversMatriseRegningVec4(const vector<float>& A, const vector<float>& b);
//void lageParabel(GLuint& VAO, GLuint& VBO);
//void tegnePunktene();
//void tegneParabel();
//void rendreScenen();
void savePointsToFile(const std::vector<glm::vec2>& points, const std::string& filename);
void lageParabel(GLuint& VAO, GLuint& VBO);
void tegnePunktene();
void tegneParabel();
void rendreScenen();


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Cube with Camera", nullptr, nullptr);
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

        rendreScenen();
        glfwPollEvents();
    }
    
    glfwTerminate();

    return 0;
}

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


//glm::vec4 inversMatriseRegningVec4(const vector<float>& A, const vector<float>& b)
//{
//    //AtA matrisen er en 4*4 matrise som tar inn verdiene til vektoren AtA. 
//    //Verdiene i denne matrisen er regnet ut med en matrisekalkulator
//    glm::mat4 AtA_matrix(A[0], A[1], A[2], A[3],
//        A[4], A[5], A[6], A[7],
//        A[8], A[9], A[10], A[11],
//        A[12], A[13], A[14], A[15]);
//
//    //Oppretter en AtY vektor som er regnet ut med matrisekalkulator
//    //Det er At verdiene ganget med y-verdi koordinatene.
//    glm::vec4 AtY_vektor(b[0], b[1], b[2], b[3]);
//
//    //Regner ut den inverse verdien til matrisen AtA og ganger resultatet med vektoren AtY 
//    glm::vec4 x = glm::inverse(AtA_matrix) * AtY_vektor;
//
//    //Returnerer løsningen til x
//    return x;
//}
//
//void lageParabel(GLuint& VAO, GLuint& VBO) {
//    // Løsning av minste kvadraters metode
//    glm::vec4 koeffisienter = inversMatriseRegningVec4(A_matrise, A_vektor);
//
//    // Parabelpunkter
//    const int punkter = 100;
//    vector<glm::vec2> points;
//    for (int i = 0; i < punkter; ++i)
//    {
//        float t = (i / (float)(punkter - 1)) * 20.0f - 10.0f;
//        float x = t;
//        float y = koeffisienter.x * x * x * x + koeffisienter.y * x * x + koeffisienter.z * x + koeffisienter.w;
//        points.push_back(glm::vec2(x, y));
//
//    }
//
//    // Generer VAO og VBO
//    glGenVertexArrays(1, &VAO);
//    glGenBuffers(1, &VBO);
//
//    // Binder VAO
//    glBindVertexArray(VAO);
//
//    // Binder VBO
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec2), &points[0], GL_STATIC_DRAW);
//
//    // Setter attributtene
//    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
//    glEnableVertexAttribArray(0);
//
//    // Unbinder VBO og VAO
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindVertexArray(0);
//}
//
//void tegnePunktene() {
//    // Punkter
//    vector<glm::vec2> points =
//    {
//        glm::vec2(1, 1), glm::vec2(2, 8), glm::vec2(6, 3), glm::vec2(8, 1)
//    };
//
//    // Lagre punktene i en tekstfil
//    savePointsToFile(points, "points.txt");
//
//     // Farger for punktene (gul)
//    vector<glm::vec3> farger(points.size(), glm::vec3(1.0f, 1.0f, 0.0f));
//
//    // Generer VAO og VBO
//    GLuint VAO, VBO, CBO;
//    glGenVertexArrays(1, &VAO);
//    glGenBuffers(1, &VBO);
//    glGenBuffers(1, &CBO);
//
//    // Binder VAO
//    glBindVertexArray(VAO);
//
//    // Binder VBO
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec2), &points[0], GL_STATIC_DRAW);
//
//    // Setter attributtene
//    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
//    glEnableVertexAttribArray(0);
//
//    // Binder CBO for farger
//    glBindBuffer(GL_ARRAY_BUFFER, CBO);
//    glBufferData(GL_ARRAY_BUFFER, farger.size() * sizeof(glm::vec3), &farger[0], GL_STATIC_DRAW);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
//    glEnableVertexAttribArray(1);
//
//    // Tegner punktene
//    glPointSize(8.0f);
//    glDrawArrays(GL_POINTS, 0, points.size());
//
//    // Unbinder VBO og VAO
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindVertexArray(0);
//
//    glDeleteBuffers(1, &VBO);
//    glDeleteBuffers(1, &CBO);
//    glDeleteVertexArrays(1, &VAO);
//}
//
//void tegneParabel() {
//    // Parabelpunkter
//    GLuint VAO, VBO;
//
//    lageParabel(VAO, VBO);
//
//    // Binder VAO
//    glBindVertexArray(VAO);
//
//    // Setter tykkelsen på linjen
//    glLineWidth(3.0f);
//
//    // Tegner parabelen
//    glDrawArrays(GL_LINE_STRIP, 0, 100);
//
//    // Unbinder VAO
//    glBindVertexArray(0);
//    glDeleteVertexArrays(1, &VAO);
//    glDeleteBuffers(1, &VBO);
//
//    // Rydde opp
//    glDeleteBuffers(1, &VBO);
//    glDeleteVertexArrays(1, &VAO);
//}
//
//void rendreScenen() {
//
//    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//    // Tegne punkter
//    tegnePunktene();
//    tegneParabel();
//    glfwSwapBuffers(window);
//}
//
//void savePointsToFile(const std::vector<glm::vec2>& points, const std::string& filename) {
//    std::ofstream file(filename);
//    if (file.is_open()) {
//        for (const auto& point : points)
//        {
//            file << point.x << " " << point.y << endl;
//        }
//        file.close();
//        cout << "Points saved to " << filename << endl;
//    }
//    else
//    {
//        cout << "Unable to open file: " << filename << endl;
//    }
//}
// Generate curve points using cubic Hermite interpolation
vector<glm::vec2> generateCurvePoints() {
    vector<glm::vec2> curvePoints;

    for (size_t i = 0; i < controlPoints.size() - 1; ++i) {
        glm::vec2 p0 = controlPoints[i];
        glm::vec2 p1 = controlPoints[i + 1];
        glm::vec2 m0 = i == 0 ? glm::normalize(p1 - p0) : 0.5f * (p1 - controlPoints[i - 1]);
        glm::vec2 m1 = i == controlPoints.size() - 2 ? glm::normalize(p1 - p0) : 0.5f * (controlPoints[i + 2] - p0);

        for (float t = 0.0f; t <= 1.0f; t += 0.01f) {
            float h00 = 2.0f * t * t * t - 3.0f * t * t + 1.0f;
            float h10 = t * t * t - 2.0f * t * t + t;
            float h01 = -2.0f * t * t * t + 3.0f * t * t;
            float h11 = t * t * t - t * t;

            glm::vec2 point = h00 * p0 + h10 * m0 + h01 * p1 + h11 * m1;
            curvePoints.push_back(point);
        }
    }

    return curvePoints;
}

void lageParabel(GLuint& VAO, GLuint& VBO) {
    // Generate curve points
    vector<glm::vec2> curvePoints = generateCurvePoints();

    // Generate VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Bind VAO
    glBindVertexArray(VAO);

    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, curvePoints.size() * sizeof(glm::vec2), curvePoints.data(), GL_STATIC_DRAW);

    // Set attributes
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void tegnePunktene() {
    // Generate VAO and VBO
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Bind VAO
    glBindVertexArray(VAO);

    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(glm::vec2), controlPoints.data(), GL_STATIC_DRAW);

    // Set attributes
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Draw points
    glPointSize(8.0f);
    glDrawArrays(GL_POINTS, 0, controlPoints.size());

    // Unbind VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Delete buffers
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void tegneParabel() {
    // Generate VAO and VBO for the curve
    GLuint VAO, VBO;
    lageParabel(VAO, VBO);

    // Bind VAO
    glBindVertexArray(VAO);

    // Set line thickness
    glLineWidth(3.0f);

    // Draw the curve
    glDrawArrays(GL_LINE_STRIP, 0, generateCurvePoints().size());

    // Unbind VAO
    glBindVertexArray(0);

    // Delete buffers
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void rendreScenen() {
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw control points
    tegnePunktene();

    // Draw the curve
    tegneParabel();

    // Swap buffers
    glfwSwapBuffers(window);
}

void savePointsToFile(const std::vector<glm::vec2>& points, const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        for (const auto& point : points)
        {
            file << point.x << " " << point.y << endl;
        }
        file.close();
        cout << "Points saved to " << filename << endl;
    }
    else
    {
        cout << "Unable to open file: " << filename << endl;
    }
}