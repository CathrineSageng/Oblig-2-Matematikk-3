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

//De 4 punktene som skal interpoleres
vector<glm::vec2> punkter = { glm::vec2(1, 1), glm::vec2(2, 8), glm::vec2(6, 3), glm::vec2(8, 1) };
//Bakgrunnsfargen
GLfloat mintGreenColor[] = { 152.0f / 255.0f, 255.0f / 255.0f, 152.0f / 255.0f };

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

void lagrePunkteneTilFil(const std::vector<glm::vec2>& points, const std::string& filename);
void lagreKoordinateneTilFil(const std::vector<glm::vec2>& points, const std::string& filename);

vector<glm::vec2> regneUtGrafKoordinatene();
void lageGraf(GLuint& VAO, GLuint& VBO);
void tegnePunktene();
void tegneGraf();
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

// Tar inn vec2 punkter som input og genererer en kurve ved å bruke kubisk hermite interpolasjon mellom punktene
vector<glm::vec2> regneUtGrafKoordinatene() 
{   
    //Denne vektoren er tom, men vil inneholde punktene som definerer kurven som skal genereres. 
    vector<glm::vec2> kurvePunkter;

    //For hvert par av punkter utføres en hermite interpolasjon for å generere flere punkter langs kurven mellom 
    //disse to punktene. Man beregner tangentvektoene i de to punktene. 
    for (size_t i = 0; i < punkter.size() - 1; ++i) 
    {
        //P0 og P1 er start og sluttpunktet for kurven som skal genereres. 
        glm::vec2 p0 = punkter[i];
        glm::vec2 p1 = punkter[i + 1];
        //Tangentvektorene, den deriverte som retningen på kurven i hvert punkt. 
        glm::vec2 m0 = i == 0 ? glm::normalize(p1 - p0) : 0.5f * (p1 - punkter[i - 1]);
        glm::vec2 m1 = i == punkter.size() - 2 ? glm::normalize(p1 - p0) : 0.5f * (punkter[i + 2] - p0);

        for (float t = 0.0f; t <= 1.0f; t += 0.01f) 
        {
            //Hermite basisfunksjoner
            float h00 = 2.0f * t * t * t - 3.0f * t * t + 1.0f;
            float h10 = t * t * t - 2.0f * t * t + t;
            float h01 = -2.0f * t * t * t + 3.0f * t * t;
            float h11 = t * t * t - t * t;

            glm::vec2 punkt = h00 * p0 + h10 * m0 + h01 * p1 + h11 * m1;
            kurvePunkter.push_back(punkt);
        }
    }

    return kurvePunkter;
}

void lageGraf(GLuint& VAO, GLuint& VBO) {
    // Generate curve points
    vector<glm::vec2> kurvePunkter = regneUtGrafKoordinatene();

    // Lagre punktene i en tekstfil
    lagreKoordinateneTilFil(kurvePunkter, "koordinater.txt");

    // Generate VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Bind VAO
    glBindVertexArray(VAO);

    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, kurvePunkter.size() * sizeof(glm::vec2), kurvePunkter.data(), GL_STATIC_DRAW);

    // Set attributes
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void tegnePunktene() 
{
        // Farger for punktene
        vector<glm::vec3> farger = 
        { glm::vec3(1.0f, 0.5f, 1.0f),
          glm::vec3(1.0f, 0.5f, 1.0f),
          glm::vec3(1.0f, 0.5f, 1.0f),
          glm::vec3(1.0f, 0.5f, 1.0f) 
        };

        // Generer VAO og VBO
        GLuint VAO, VBO, CBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &CBO);

        // Binder VAO
        glBindVertexArray(VAO);

        // Binder VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, punkter.size() * sizeof(glm::vec2), punkter.data(), GL_STATIC_DRAW);

        // Setter attributtene for posisjon
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Binder CBO for farger
        glBindBuffer(GL_ARRAY_BUFFER, CBO);
        glBufferData(GL_ARRAY_BUFFER, farger.size() * sizeof(glm::vec3), farger.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);

        // Tegner punktene
        glPointSize(12.0f);
        glDrawArrays(GL_POINTS, 0, punkter.size());

        // Unbinder VBO og VAO
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Sletter bufferne
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &CBO);
        glDeleteVertexArrays(1, &VAO);

    // Lagre punktene i en tekstfil
    lagrePunkteneTilFil(punkter, "punkter.txt");
}

void tegneGraf() {
    // Generate VAO and VBO for the curve
    GLuint VAO, VBO;
    lageGraf(VAO, VBO);

    // Bind VAO
    glBindVertexArray(VAO);

    // Set line thickness
    glLineWidth(3.0f);

    // Draw the curve
    glDrawArrays(GL_LINE_STRIP, 0, regneUtGrafKoordinatene().size());

    // Unbind VAO
    glBindVertexArray(0);

    // Delete buffers
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void rendreScenen() {
    glClearColor(mintGreenColor[0], mintGreenColor[1], mintGreenColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Tegner de 4 punktene
    tegnePunktene();

    // Tegner grafen
    tegneGraf();

    // Swap buffers
    glfwSwapBuffers(window);
}

void lagrePunkteneTilFil(const std::vector<glm::vec2>& punkter, const std::string& filnavn) {
    std::ofstream file(filnavn);
    if (file.is_open()) {
        for (const auto& point : punkter)
        {
            file << point.x << " " << point.y << endl;
        }
        file.close();
        cout << "Punktene lagret i filen:  " << filnavn << endl;
    }
    else
    {
        cout << "Klarte ikke å opne tekstfilen: " << filnavn << endl;
    }
}

void lagreKoordinateneTilFil(const std::vector<glm::vec2>& punkter, const std::string& filnavn) {
    std::ofstream file(filnavn);
    if (file.is_open()) {
        for (const auto& point : punkter)
        {
            file << "x: " << point.x << " ,y:" << point.y << endl;
        }
        file.close();
        cout << "Punktene lagret i filen: " << filnavn << endl;
    }
    else
    {
        cout << "Klarte ikke å opne tekstfilen: " << filnavn << endl;
    }
}