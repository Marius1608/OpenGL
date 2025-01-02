#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <C:\Users\pante\Desktop\An3_sem1\PG\Biblioteci\OpenGL dev libs\include\GL/glew.h>
#endif

#include <C:\Users\pante\Desktop\An3_sem1\PG\Biblioteci\OpenGL dev libs\include\GLFW/glfw3.h>

#include <C:\Users\pante\Desktop\An3_sem1\PG\Biblioteci\glm/glm.hpp> //core glm functionality
#include <C:\Users\pante\Desktop\An3_sem1\PG\Biblioteci\glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <C:\Users\pante\Desktop\An3_sem1\PG\Biblioteci\glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <C:\Users\pante\Desktop\An3_sem1\PG\Biblioteci\glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

GLint densityLoc;
GLint gradientLoc;
GLint fogColorLoc;

GLuint depthMapFBO, depthMap;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
gps::Shader depthMapShader;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 2.0f, 5.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

gps::Shader myBasicShader;

gps::Model3D stadium;
gps::Model3D ball;
glm::vec3 ballPosition = glm::vec3(0.0f, 1.0f, 0.0f);

float angle = 0.0f;

float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch = 0.0f;
bool firstMouse = true;

bool mouseEnabled = true;

GLenum polygonMode = GL_FILL;
bool isAnimating = false;
float ballVerticalSpeed = 0.0f;
const float GRAVITY = -9.81f;
const float BOUNCE_FACTOR = 0.7f;
float lastFrameTime = 0.0f;

bool fogEnabled = false;


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

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        fogEnabled = !fogEnabled;
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            pressedKeys[key] = true;
        else if (action == GLFW_RELEASE)
            pressedKeys[key] = false;
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

    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        polygonMode = (polygonMode == GL_FILL) ? GL_LINE :
            (polygonMode == GL_LINE) ? GL_POINT : GL_FILL;
        glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
    }

    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        isAnimating = !isAnimating;
        if (isAnimating) {
            ballVerticalSpeed = 5.0f; // Initial upward velocity
        }
    }

}

void updateBallPosition() {
    if (!isAnimating) return;

    float currentTime = glfwGetTime();
    float deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;

    ballVerticalSpeed += GRAVITY * deltaTime;
    ballPosition.y += ballVerticalSpeed * deltaTime;

    // Ground collision
    if (ballPosition.y <= 0.2f) {
        ballPosition.y = 0.2f;
        ballVerticalSpeed = -ballVerticalSpeed * BOUNCE_FACTOR;

        if (abs(ballVerticalSpeed) < 0.1f) {
            ballVerticalSpeed = 0.0f;
            isAnimating = false;
        }
    }
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

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f); // Sky blue
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_MULTISAMPLE);
}

void initModels() {
    stadium.LoadModel("models/arena/scenes/arena.obj");
    ball.LoadModel("models/ball/ball.obj");
}

void initShaders() {
    depthMapShader.loadShader("shaders/shadowMap.vert", "shaders/shadowMap.frag");
    myBasicShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
}


void initShadowMapping() {
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
}

void initUniforms() {
    myBasicShader.useShaderProgram();

    // Initialize model matrix
    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Initialize view matrix
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // Initialize normal matrix
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // Set projection matrix
    projection = glm::perspective(glm::radians(60.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    // Adjust light direction for stadium lighting
    lightDir = glm::vec3(-0.5f, 1.0f, -0.5f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::normalize(lightDir)));

    lightColor = glm::vec3(1.0f, 0.95f, 0.8f);
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    densityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "density");
    gradientLoc = glGetUniformLocation(myBasicShader.shaderProgram, "gradient");
    fogColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogColor");
}


void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myBasicShader.useShaderProgram();
   

    if (fogEnabled) {
        glUniform1f(densityLoc, 0.007f);
        glUniform1f(gradientLoc, 1.5f);
        glUniform3f(fogColorLoc, 0.5f, 0.6f, 0.7f);
    }
    else {
        glUniform1f(densityLoc, 0.0f);
        glUniform1f(gradientLoc, 0.0f);
    }

    // Render stadium
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, -10.0f));
    model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    stadium.Draw(myBasicShader);

    // Render ball
    model = glm::mat4(1.0f);
    model = glm::translate(model, ballPosition);
    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    ball.Draw(myBasicShader);

	// Render depth map
	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 20.0f) * glm::lookAt(-lightDir, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0))));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	ball.Draw(depthMapShader);
	stadium.Draw(depthMapShader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);


      
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

    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
        renderScene();
        updateBallPosition();
        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();
    return EXIT_SUCCESS;
}