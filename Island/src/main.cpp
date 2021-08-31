//#ifdef TEST
#include <iostream>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "water.h"
#include "skybox.h"
#include "resource_manager.h"
#include "camera.h"
#include "terrain.h"
#include "model.h"
#include "light.h"
#include "fog.h"
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
float waterHeight = 2.f;

//camera data for generating view matrix
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float pitch{ 0.0f }, yaw{ -90.0f };
float fov{ 45.0f };

// Light
const glm::vec3 lightDir = glm::normalize(glm::vec3(0.43f, 0.76f, -0.33f));//(0.6f, 0.76f, -0.25f)
const glm::vec3 lightColor = glm::vec3(1.f, 1.f, 0.7f);
const GLfloat ambientStrength = 0.08;
const glm::vec3 ambientColor = glm::vec3(0.25f, 0.25f, 1.f);

// Fog
const float fogDensity = 0.0045f;
const glm::vec3 fogColor1 = glm::vec3(0.604f, 0.655f, 0.718f);
const glm::vec3 fogColor2 = glm::vec3(0.631f, 0.651f, 0.698f);

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
    glfwWindowHint(GLFW_RESIZABLE, false);

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
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Display waiting information while program is loading up
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Shader loadingShader = ResourceManager::loadShader("shaders/post_processing.vs", "shaders/loading.fs", nullptr, "quad");
    Texture2D loadingTexture = ResourceManager::loadTexture("resources/textures/LoadingPicture.png", true, "lensstar");
    loadingShader.setInteger("loadingPicture", 0, true);
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
    Shader shaderTerrain = ResourceManager::loadShader("shaders/terrain.vs", "shaders/terrain.fs", nullptr, "shaderTerrain");
    Shader SimpleShader = ResourceManager::loadShader("shaders/simple.vs", "shaders/simple.fs", nullptr, "SimpleShader");
    //Shader sunShader = ResourceManager::loadShader("shaders/sun.vs", "shaders/sun.fs", nullptr, "quad");

    // Load Textures
    Texture2D textureTerrain = ResourceManager::loadTexture("resources/textures/grass_COLOR.png", false, "textureTerrain");
    Texture2D SunTexture = ResourceManager::loadTexture("resources/textures/sun.png", true, "lensstar");

    // Configure Texture Samplers
    shaderTerrain.setInteger("terrain", 0, true);
    shaderTerrain.setInteger("shadowMap", 1);

    // Terrain
    Terrain terrain;
    terrain.load(terrainWaterSize, 37.5f, 300.0f, "resources/textures/heightmap_island_low_poly.jpg");
    camera.loadTerrain(&terrain);

    // Water
    Water water(terrainWaterSize, waterHeight);
    water.load("resources/textures/water_dudv_blur.jpg", "resources/textures/water_normal.jpg", 100.f);

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
    GLuint const SHADOW_RESOLUTION = 4096; //8192;//
    GLuint ShadowFBO;
    Texture2D shadowDepth;
    shadowDepth.Image_Format = GL_DEPTH_COMPONENT;
    shadowDepth.Internal_Format = GL_DEPTH_COMPONENT;
    shadowDepth.Filter_Min = GL_LINEAR;
    // Manually set depth attachment to clamp to border value of 1.0 depth; ensures areas that are not visible from light are not in shadow.
    shadowDepth.Wrap_S = GL_CLAMP_TO_BORDER;
    shadowDepth.Wrap_T = GL_CLAMP_TO_BORDER;
    shadowDepth.generate(SHADOW_RESOLUTION, SHADOW_RESOLUTION, NULL);
    shadowDepth.bind();
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glGenFramebuffers(1, &ShadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ShadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepth.ID, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::SHADOW_FRAMEBUFFER" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Set Projection Matrix
    glm::mat4 projection = camera.SetProjectionMatrix((float)SCR_WIDTH, (float)SCR_HEIGHT, near, far); // this remains unchanged for every frame
    shaderTerrain.setMatrix4("projection", projection, true);
    water.m_shader.setMatrix4("projection", projection, true);
    //sunShader.setMatrix4("projection", projection, true);

    // Sun
    Light sun(lightDir, lightColor, ambientStrength, ambientColor);
    sun.setShader(shaderTerrain, "sun", true);
    sun.setShader(water.m_shader, "sun", true);

    // Fog
    Fog fog(fogDensity, fogColor1);
    fog.setShader(shaderTerrain, "fog", true);
    fog.setShader(water.m_shader, "fog", true);
    fog.setShader(shaderSkybox, "fog", true);

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

#pragma region SHADOW
        /////////////////////////////////////////////////////////
        ///////////////////  SHADOW PASS  ///////////////////////
        /////////////////////////////////////////////////////////

        glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
        glBindFramebuffer(GL_FRAMEBUFFER, ShadowFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        glm::mat4 lightMatrix;
        GLfloat orthoWidth = 150.0f;
        glm::mat4 lightProjection = glm::ortho(-orthoWidth, orthoWidth, -orthoWidth, orthoWidth, 25.0f, 350.0f);
        glm::mat4 lightView = glm::lookAt(sun.m_direction * 250.f + glm::vec3(camera.Position.x, 0.0f, camera.Position.z),
                                        glm::vec3(0.0f) + glm::vec3(camera.Position.x, 0.0f, camera.Position.z),
                                        glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 biasMatrix = glm::mat4();
        biasMatrix[0][0] = 0.5; biasMatrix[0][1] = 0.0; biasMatrix[0][2] = 0.0; biasMatrix[0][3] = 0.0;
        biasMatrix[1][0] = 0.0; biasMatrix[1][1] = 0.5; biasMatrix[1][2] = 0.0; biasMatrix[1][3] = 0.0;
        biasMatrix[2][0] = 0.0; biasMatrix[2][1] = 0.0; biasMatrix[2][2] = 0.5; biasMatrix[2][3] = 0.0;
        biasMatrix[3][0] = 0.5; biasMatrix[3][1] = 0.5; biasMatrix[3][2] = 0.5; biasMatrix[3][3] = 1.0;
        lightMatrix = lightProjection * lightView; // Render texture as full texture, add bias only to final matrix (to reposition vertex to 0.0 - 1.0f space)

        /***********************Terrain*********************/
        SimpleShader.use();
        SimpleShader.setMatrix4("lightMatrix", lightMatrix);
        SimpleShader.setMatrix4("model", glm::mat4());
        terrain.render();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        lightMatrix = biasMatrix * lightMatrix; // Add bias to lightMatrix (convert NDC to [0.0, 1.0] interval)

#pragma endregion SHADOW

#pragma region REFRACTION
        ///////////////////////////////////////////////////////////
        ///////////////////  REFRACTION PASS  /////////////////////
        ///////////////////////////////////////////////////////////

        water.initPassRefraction();

        /**********************Terrain********************/
        shaderTerrain.setMatrix4("view", view, GL_TRUE);
        shaderTerrain.setInteger("isRefraction", GL_TRUE);
        shaderTerrain.setInteger("isReflection", GL_FALSE);
        shaderTerrain.setFloat("waterHeight", water.getHeight());
        shaderTerrain.setMatrix4("shadowMat", lightMatrix);
        shaderTerrain.setVector3f("viewPos", camera.Position);
        textureTerrain.bind(0);
        shadowDepth.bind(1);
        terrain.render();

        water.terminatePassRefraction();

#pragma endregion REFRACTION

#pragma region REFLECTION
        ///////////////////////////////////////////////////////////
        ///////////////////  REFLECTION PASS  /////////////////////
        ///////////////////////////////////////////////////////////

        // now use a imaginary camera on the counter position under watersurface
        glm::mat4 imgView = camera.GetImaginaryViewMatrix(water.getHeight());

        water.initPassReflection();

        /**********************Terrain********************/
        shaderTerrain.setMatrix4("view", imgView, GL_TRUE);
        shaderTerrain.setInteger("isRefraction", GL_FALSE);
        shaderTerrain.setInteger("isReflection", GL_TRUE);
        shaderTerrain.setFloat("waterHeight", water.getHeight());
        shaderTerrain.setMatrix4("shadowMat", lightMatrix);
        shaderTerrain.setVector3f("viewPos", camera.Position);
        textureTerrain.bind(0);
        shadowDepth.bind(1);
        terrain.render();

        /***********************Skybox*********************/
        skybox.render(imgView, projection, 1.f);

        water.terminatePassReflection();

#pragma endregion REFLECTION

        /*
        // Check if the sun is in our sight. If not, skip GOD RAYS pass and POST PROCESSING pass.
        glm::vec4 position = projection * glm::mat4(glm::mat3(view)) * glm::vec4(sun.m_direction * 250.f, 1.f);
        float zValue = position.z;
        glm::vec2 sunPos = glm::vec2(position.x, position.y);
        sunPos /= position.w;
        sunPos = (sunPos * 0.5f) + 0.5f;
        bool doGodRays = true;
        const float margin = 0.05f;
        if (zValue < 0.f || sunPos.x < 0.f - margin || sunPos.x > 1.f + margin || sunPos.y < 0.f - margin || sunPos.y > 1.f + margin)//the interval should be slightly larger than [0, 1], otherwise when the sun appear from the edges of the screen it looks bad
            doGodRays = false;
        */

#pragma region GOD_RAYS
        ///////////////////////////////////////////////////////////
        ///////////////////////  GOD RAYS  ////////////////////////
        ///////////////////////////////////////////////////////////
#pragma endregion GOD_RAYS

#pragma region NORMAL
        ///////////////////////////////////////////////////////////
        /////////////////////  NORMAL PASS  ///////////////////////
        ///////////////////////////////////////////////////////////
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /**********************Terrain********************/
        shaderTerrain.setMatrix4("view", view, GL_TRUE);
        shaderTerrain.setInteger("isRefraction", GL_FALSE);
        shaderTerrain.setInteger("isReflection", GL_FALSE);
        shaderTerrain.setMatrix4("shadowMat", lightMatrix);
        shaderTerrain.setVector3f("viewPos", camera.Position);
        textureTerrain.bind(0);
        shadowDepth.bind(1);
        terrain.render();

        /***********************Water*********************/
        water.m_shader.setMatrix4("view", view, GL_TRUE);
        water.m_shader.setFloat("time", glfwGetTime() / 10.f);
        water.m_shader.setVector3f("viewPos", camera.Position);
        water.render();

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
//#endif