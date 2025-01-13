#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <C:\Users\pante\Desktop\An3_sem1\PG\Biblioteci\OpenGL dev libs\include\GL/glew.h>
#endif

#include <C:\Users\pante\Desktop\An3_sem1\PG\Biblioteci\OpenGL dev libs\include\GLFW/glfw3.h>

#include <C:\Users\pante\Desktop\An3_sem1\PG\Biblioteci\glm/glm.hpp>
#include <C:\Users\pante\Desktop\An3_sem1\PG\Biblioteci\glm/gtc/matrix_transform.hpp>
#include <C:\Users\pante\Desktop\An3_sem1\PG\Biblioteci\glm/gtc/matrix_inverse.hpp>
#include <C:\Users\pante\Desktop\An3_sem1\PG\Biblioteci\glm/gtc/type_ptr.hpp>

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>

gps::Window myWindow;

glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

glm::vec3 lightDir;
glm::vec3 lightColor;


GLint diffuseTexture;
GLint metallicTexture;
GLint roughnessTexture;


glm::vec3 spotLightPosition = glm::vec3(0.0f, 10.0f, 0.0f);
glm::vec3 spotLightDirection = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 spotLightColor = glm::vec3(1.0f, 1.0f, 0.8f);
float spotLightCutOff = glm::cos(glm::radians(12.5f));
float spotLightOuterCutOff = glm::cos(glm::radians(17.5f));


GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

GLint densityLoc;
GLint gradientLoc;
GLint fogColorLoc;
GLint isWhiteLoc;

float scaleFactorScene = 1.0f;
bool presentationMode = false;
float presentationAngle = 0.0f;
const float PRESENTATION_RADIUS = 15.0f;
const float PRESENTATION_HEIGHT = 5.0f;
const float PRESENTATION_SPEED = 0.5f;

struct RainDrop {
    glm::vec3 position;
    float speed;
    float length;
};
const int MAX_RAINDROPS = 1000;
std::vector<RainDrop> raindrops;
bool rainEnabled = false;
GLuint rainVAO, rainVBO;
gps::Shader rainShader;


enum RenderMode {
    SOLID,
    WIREFRAME,
    POINT,
    SMOOTH
};
RenderMode currentRenderMode = SOLID;

GLuint depthMapFBO, depthMap;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
gps::Shader depthMapShader;

gps::Camera myCamera(
    glm::vec3(0.0f, 2.0f, 5.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

gps::Shader myBasicShader;
gps::Shader ballShader;

gps::Model3D stadium;
gps::Model3D ball;
gps::Model3D goalkeeper;
gps::Model3D reflector1;
gps::Model3D reflector2;

glm::vec3 ballPosition = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 goalkeeperPosition = glm::vec3(0.0f, 0.0f, -15.0f);

float goalkeeperRotation = 0.0f;
bool isDiving = false;
float diveProgress = 0.0f;
float diveDirection = 0.0f;

float angle = 0.0f;
float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch = 0.0f;
bool firstMouse = true;
bool mouseEnabled = true;
bool fogEnabled = false;

GLenum polygonMode = GL_FILL;
bool isAnimating = false;
float ballVerticalSpeed = 0.0f;
const float GRAVITY = -9.81f;
const float BOUNCE_FACTOR = 0.7f;
float lastFrameTime = 0.0f;


void initRain() {

    raindrops.clear();
    
    for (int i = 0; i < MAX_RAINDROPS; i++) {

        RainDrop drop;
        drop.position = glm::vec3(
            (rand() % 200 - 100) * 0.5f, 
            (rand() % 100) * 0.5f + 20.0f, 
            (rand() % 200 - 100) * 0.5f 
        );
        
        drop.speed = 15.0f + static_cast<float>(rand()) / RAND_MAX * 10.0f;
        drop.length = 0.2f + static_cast<float>(rand()) / RAND_MAX * 0.3f;
        raindrops.push_back(drop);
    }

    glGenVertexArrays(1, &rainVAO);
    glGenBuffers(1, &rainVBO);

    std::vector<glm::vec3> initialPositions;
    for (const auto& drop : raindrops) {
        initialPositions.push_back(drop.position);
    }

    glBindVertexArray(rainVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rainVBO);
    glBufferData(GL_ARRAY_BUFFER, initialPositions.size() * sizeof(glm::vec3), initialPositions.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void updateRain(float deltaTime) {

    if (!rainEnabled) return;

    std::vector<glm::vec3> positions;
    positions.reserve(raindrops.size());

    for (auto& drop : raindrops) {
       
        drop.position.y -= drop.speed * deltaTime;

        if (drop.position.y < 0.0f) {
            drop.position.y = 50.0f + static_cast<float>(rand()) / RAND_MAX * 20.0f;
            drop.position.x = (rand() % 200 - 100) * 0.5f;
            drop.position.z = (rand() % 200 - 100) * 0.5f;
            drop.speed = 15.0f + static_cast<float>(rand()) / RAND_MAX * 10.0f;
        }

        positions.push_back(drop.position);
    }

    glBindBuffer(GL_ARRAY_BUFFER, rainVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void renderRain() {

    if (!rainEnabled) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_DEPTH_TEST);

    rainShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

    glBindVertexArray(rainVAO);
    glDrawArrays(GL_POINTS, 0, MAX_RAINDROPS);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_BLEND);
}

void updateGoalkeeper() {

    const float DIVE_HEIGHT = 2.5f;
    const float DIVE_DISTANCE = 3.0f;
    const float DIVE_DURATION = 1.0f;
    static bool alternateDirection = false;

    if (ballPosition.z < -10.0f && !isDiving) {
        isDiving = true;
        diveProgress = 0.0f;
        diveDirection = (ballPosition.x > 0) ? 1.0f : -1.0f;
        alternateDirection = !alternateDirection;
        diveDirection = alternateDirection ? 1.0f : -1.0f;
        goalkeeperRotation = diveDirection * 45.0f;
    }

    if (isDiving) {
        diveProgress += 0.016f; 
        if (diveProgress >= DIVE_DURATION) {
            isDiving = false;
            goalkeeperPosition = glm::vec3(0.0f, 0.0f, -15.0f);
            goalkeeperRotation = 0.0f;
        }
        else {
            float t = diveProgress / DIVE_DURATION;
            goalkeeperPosition.y = DIVE_HEIGHT * sin(t * 3.14159f);
            goalkeeperPosition.x = diveDirection * DIVE_DISTANCE * t;
            goalkeeperRotation = diveDirection * (45.0f + t * 45.0f);
        }
    }
}

void updateBallPosition() {

    static bool isMovingToGoal = false;
    static glm::vec3 initialPosition;
    static float jumpTimer = 0.0f;
    const float MOVE_SPEED = 0.2f;
    const float JUMP_HEIGHT = 2.0f;
    const float JUMP_DURATION = 1.0f;

    if (!isAnimating) return;

    float currentTime = glfwGetTime();
    float deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;

    ballVerticalSpeed += GRAVITY * deltaTime;
    ballPosition.y += ballVerticalSpeed * deltaTime;

    if (ballPosition.y <= 0.2f) {
        ballPosition.y = 0.2f;
        ballVerticalSpeed = -ballVerticalSpeed * BOUNCE_FACTOR;

        if (abs(ballVerticalSpeed) < 0.1f) {
            ballVerticalSpeed = 0.0f;
            isAnimating = false;
        }
    }

    if (pressedKeys[GLFW_KEY_V] && !isMovingToGoal) {
        isMovingToGoal = true;
        initialPosition = ballPosition;
        jumpTimer = 0.0f;
        ballVerticalSpeed = 5.0f;
    }

    if (isMovingToGoal) {
        
        ballPosition.z -= MOVE_SPEED;

        jumpTimer += 0.016f; 
        if (jumpTimer <= JUMP_DURATION) {
            float jumpProgress = jumpTimer / JUMP_DURATION;
            ballPosition.y = initialPosition.y + JUMP_HEIGHT * sin(jumpProgress * 3.14159f);
        }

        if (ballPosition.z <= -15.0f) {
            isMovingToGoal = false;
            ballPosition = glm::vec3(0.0f, 1.0f, 0.0f); 
        }
    }
}

void updatePresentationMode() {

    if (presentationMode) {
        presentationAngle += PRESENTATION_SPEED;
        if (presentationAngle >= 360.0f) {
            presentationAngle -= 360.0f;
        }

        float camX = PRESENTATION_RADIUS * sin(glm::radians(presentationAngle));
        float camZ = PRESENTATION_RADIUS * cos(glm::radians(presentationAngle));

        glm::vec3 newPosition = glm::vec3(camX, PRESENTATION_HEIGHT, camZ);
        glm::vec3 newTarget = glm::vec3(0.0f, 0.0f, 0.0f);

        myCamera.rotate(glm::degrees(asin(newPosition.y / glm::length(newPosition))),
            glm::degrees(atan2(newPosition.x, newPosition.z)));

        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }
}


GLenum glCheckError_(const char* file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM: error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE: error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION: error = "INVALID_OPERATION"; break;
        case GL_OUT_OF_MEMORY: error = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    WindowDimensions dims;
    dims.width = width;
    dims.height = height;
    myWindow.setWindowDimensions(dims);
    glViewport(0, 0, width, height);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    scaleFactorScene += yoffset * 0.1f;
    scaleFactorScene = glm::clamp(scaleFactorScene, 0.1f, 3.0f);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

    if (!mouseEnabled) return;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
}


void processMovement() {

    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {

    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        if (abs(ballPosition.z - goalkeeperPosition.z) < 5.0f) {
            isDiving = true;
            diveProgress = 0.0f;
            diveDirection = (ballPosition.x > goalkeeperPosition.x) ? 1.0f : -1.0f;
        }
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        fogEnabled = !fogEnabled;
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        rainEnabled = !rainEnabled;
    }

    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        mouseEnabled = !mouseEnabled;
        if (mouseEnabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    if (key == GLFW_KEY_O && action == GLFW_PRESS) {
        presentationMode = !presentationMode;
        if (presentationMode) {
            mouseEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else {
            mouseEnabled = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        isAnimating = !isAnimating;
        if (isAnimating) {
            ballVerticalSpeed = 5.0f; 
        }
    }

    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        switch (currentRenderMode) {
        case SOLID:
            currentRenderMode = WIREFRAME;
            polygonMode = GL_LINE;
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case WIREFRAME:
            currentRenderMode = POINT;
            polygonMode = GL_POINT;
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            break;
        case POINT:
            currentRenderMode = SMOOTH;
            polygonMode = GL_FILL;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_POLYGON_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
            glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
            break;
        case SMOOTH:
            currentRenderMode = SOLID;
            polygonMode = GL_FILL;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_LINE_SMOOTH);
            glDisable(GL_POLYGON_SMOOTH);
            break;
        }
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        spotLightPosition.x += 1.0f;
        myBasicShader.useShaderProgram();
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLightPosition"), 1, glm::value_ptr(spotLightPosition));
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        spotLightPosition.x -= 1.0f;
        myBasicShader.useShaderProgram();
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLightPosition"), 1, glm::value_ptr(spotLightPosition));
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        spotLightPosition.y += 1.0f;
        myBasicShader.useShaderProgram();
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLightPosition"), 1, glm::value_ptr(spotLightPosition));
    }
    if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
        spotLightPosition.y -= 1.0f;
        myBasicShader.useShaderProgram();
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLightPosition"), 1, glm::value_ptr(spotLightPosition));
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            pressedKeys[key] = true;
        else if (action == GLFW_RELEASE)
            pressedKeys[key] = false;
    }
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetScrollCallback(myWindow.getWindow(), scroll_callback);
}


void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initOpenGLState() {

    glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_MULTISAMPLE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

    glPointSize(5.0f);

    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, metallicTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, roughnessTexture);
}

void initModels() {
    stadium.LoadModel("models/arena/scenes/arena.obj");
    ball.LoadModel("models/ball/ball.obj");
    goalkeeper.LoadModel("models/goalkeeper/goalkeeper.obj");
    initRain();
}

void initShaders() {
    depthMapShader.loadShader("shaders/shadowMap.vert", "shaders/shadowMap.frag");
    myBasicShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
    ballShader.loadShader("shaders/ball.vert", "shaders/ball.frag");
    rainShader.loadShader("shaders/rain.vert", "shaders/rain.frag");
}

void initUniforms() {

    myBasicShader.useShaderProgram();

    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    projection = glm::perspective(glm::radians(60.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));



    lightDir = glm::vec3(-0.5f, 1.0f, -0.5f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::normalize(lightDir)));

    lightColor = glm::vec3(1.0f, 0.95f, 0.8f);
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    

    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLightPosition"), 1, glm::value_ptr(spotLightPosition));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLightDirection"), 1, glm::value_ptr(spotLightDirection));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLightColor"), 1, glm::value_ptr(spotLightColor));
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLightCutOff"), spotLightCutOff);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLightOuterCutOff"), spotLightOuterCutOff);

    

    densityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "density");
    gradientLoc = glGetUniformLocation(myBasicShader.shaderProgram, "gradient");
    fogColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogColor");
    isWhiteLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isWhite");

    
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "diffuseTexture"), 0);  
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 1);       
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "metallicTexture"), 2); 
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "roughnessTexture"), 3);

    
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "usePBR"), true);

   
    glm::vec3 viewPos = glm::vec3(0.0f, 2.0f, 5.0f); 
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "viewPos"), 1, glm::value_ptr(viewPos));

}


void renderScene() {
    
    depthMapShader.useShaderProgram();

    glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.1f, 50.0f);
    glm::mat4 lightView = glm::lookAt(-lightDir * 15.0f, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
        1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, -10.0f));
    model = glm::scale(model, glm::vec3(0.1f * scaleFactorScene));
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
        1, GL_FALSE, glm::value_ptr(model));
    stadium.Draw(depthMapShader);
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, ballPosition);
    model = glm::scale(model, glm::vec3(0.2f * scaleFactorScene));
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
        1, GL_FALSE, glm::value_ptr(model));
    ball.Draw(depthMapShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, goalkeeperPosition);
    model = glm::rotate(model, glm::radians(goalkeeperRotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(7.0f * scaleFactorScene));
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
        1, GL_FALSE, glm::value_ptr(model));
    goalkeeper.Draw(depthMapShader);


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);



    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    myBasicShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceMatrix"),
        1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 1);

    if (fogEnabled) {
        glUniform1f(densityLoc, 0.007f);
        glUniform1f(gradientLoc, 1.5f);
        glUniform3f(fogColorLoc, 0.5f, 0.6f, 0.7f);
    }
    else {
        glUniform1f(densityLoc, 0.0f);
        glUniform1f(gradientLoc, 0.0f);
    }

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, -10.0f));
    model = glm::scale(model, glm::vec3(0.1f * scaleFactorScene));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    stadium.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, ballPosition);
    model = glm::scale(model, glm::vec3(0.2f * scaleFactorScene));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(isWhiteLoc, 1);
    ball.Draw(myBasicShader);
    glUniform1i(isWhiteLoc, 0);

    model = glm::mat4(1.0f);
    model = glm::translate(model, goalkeeperPosition);
    model = glm::rotate(model, glm::radians(goalkeeperRotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(7.0f * scaleFactorScene));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(isWhiteLoc, 1);
    goalkeeper.Draw(myBasicShader);
    glUniform1i(isWhiteLoc, 0);

    renderRain();
}

void cleanup() {
    myWindow.Delete();
}

int main(int argc, const char* argv[]) {

    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
    initShaders();
    initUniforms();
    setWindowCallbacks();

    glCheckError();

    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processMovement();
        updatePresentationMode();
        updateRain(deltaTime);
        updateGoalkeeper();
        renderScene();
        updateBallPosition();
        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();
    return EXIT_SUCCESS;
}