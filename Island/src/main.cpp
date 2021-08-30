#include <iostream>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "skybox.h"
#include "resource_manager.h"
#include "camera.h"
#include "model.h"
#include "framebuffer.h"
#include "geometry.h"

Camera camera(glm::vec3(0.0f, 10.0f, 0.0f));

// settings
const GLuint SCR_WIDTH = 1280;
const GLuint SCR_HEIGHT = 720;
const float near = 0.1f;
const float far = 750.0f;
float deltaTime{ 0.0f };
glm::vec2 terrainWaterSize = glm::vec2(750.0f);

//camera data for generating view matrix
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float pitch{ 0.0f }, yaw{ -90.0f };
float fov{ 45.0f };

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

bool cursorFlag{ false };

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    //glfwWindowHint(GLFW_RESIZABLE, false);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Island", nullptr, nullptr);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set Window
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Display waiting information while program is loading up
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Shader loadingShader = ResourceManager::loadShader("shaders/post_processing.vs", "shaders/loading.fs", nullptr, "quad");
    Texture2D loadingTexture = ResourceManager::loadTexture("resources/textures/LoadingPicture.png", true, "lensstar");
    loadingShader.setInteger("loadingPicture", 0, GL_TRUE);
    loadingTexture.bind(0);
    Geometry::drawPlane();
    glfwSwapBuffers(window);

    // Load Models
    //Model house("models/house/farmhouse.obj");
    //house.calculateBoundingVolume();
    //Model tree("models/tree3/laubbaum.obj");
    //tree.calculateBoundingVolume();

    // Load Shaders
    Shader shaderSkybox = ResourceManager::loadShader("shaders/skybox.vs", "shaders/skybox.fs", nullptr, "shaderSkybox");

    // Load Textures

    // Configure Texture Samplers

    // Terrain
    //Terrain terrain;
    //terrain.load(terrainWaterSize, 37.5f, 300.0f, "resources/textures/heightmap_island_low_poly.jpg");
    //camera.loadTerrain(&terrain);

    // Water

    // Trees

    // Houses

    // Skybox
    Skybox skybox(&shaderSkybox);

    skybox.loadDiurnalSkybox(
        "resources/skybox/day/right.png",
        "resources/skybox/day/left.png",
        "resources/skybox/day/top.png",
        "resources/skybox/day/bottom.png",
        "resources/skybox/day/back.png",
        "resources/skybox/day/front.png"
    );

    skybox.loadNocturnalSkybox(
        "resources/skybox/night/right.png",
        "resources/skybox/night/left.png",
        "resources/skybox/night/top.png",
        "resources/skybox/night/bottom.png",
        "resources/skybox/night/back.png",
        "resources/skybox/night/front.png"
    );

    // Shadow framebuffer

    // Set Projection Matrix
    glm::mat4 projection = camera.SetProjectionMatrix((float)SCR_WIDTH, (float)SCR_HEIGHT, near, far); // this remains unchanged for every frame

    // Sun

    // Fog

    // Trees - Instanced array

    // Render to Texture

    // deltaTime variables
    float lastTime{ 0.0f };
    float UpdateTime{ 0.0f };

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        UpdateTime += deltaTime;

        if (UpdateTime > 1.f) {
            UpdateTime = 0.f;
            glfwSetWindowTitle(window, ("Island  FPS : " + std::to_string(1.f / deltaTime)).c_str());
        }

        processInput(window);

        // configure view matrices
        glm::mat4 view = camera.GetViewMatrix();
        camera.CalculateViewFrustum();
        glm::mat4 matProjectionView = projection * view;

        // cull trees that are out of frustum

#pragma region NORMAL
        ///////////////////////////////////////////////////////////
        /////////////////////  NORMAL PASS  ///////////////////////
        ///////////////////////////////////////////////////////////
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /***********************Skybox*********************/
        skybox.render(view, projection, 1.f);

#pragma endregion NORMAL

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ResourceManager::clear();
    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //move camera
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cursorFlag = !cursorFlag;
        if (cursorFlag)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float offsetX = xpos - lastX;
    float offsetY = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(offsetX, offsetY);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}