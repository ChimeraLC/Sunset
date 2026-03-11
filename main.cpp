#include <stdlib.h>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <unistd.h>

#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <format>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "model.h"
#include "shader.h"
#include "camera.h"
#include "debug.h"

#include "main.h"

using namespace std;
using namespace glm;

// Config settings
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;
int curWidth = SCREEN_WIDTH;
int curHeight = SCREEN_HEIGHT;
const float ASPECT_RATIO = (float) SCREEN_WIDTH / (float) SCREEN_HEIGHT;
const float FRAMERATE = 60.0f;
const float PER_FRAME = 1 / FRAMERATE;

const int SHADOW_RESOLUTION = 512;

const float COLOR_WHITE[] = {1.0, 1.0, 1.0, 1.0};
const float COLOR_BLACK[] = {0.0, 0.0, 0.0, 1.0};
vec3 sunDirection = vec3(0, 1, 0);
mat4 sunTransform = mat4(1.0);
float sunShadowDist = 5;    // Distance shadowmap is rendered from
float sunRenderDist = 15;   // Distance sun model is rendered

// Global values
const int bufferCount = 4;
unsigned int VAOs[bufferCount];
vector<int> triangleCounts;
vector<ModelData> modelDatas;

Camera* camera;
float mouseX;
float mouseY;
bool mouseSet;

int main(int argc, char *argv[])
{
    if (!handleArgs(argc, argv))
    {
        return -1;
    }

    if (DebugActive(DEBUG_VERBOSE))
    {
        cout << "Running with verbose logs" << endl;
        cout << "Screen size set to: " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << endl;
        cout << "Shadowmap resolution set to: " << SHADOW_RESOLUTION << endl;
        cout << "Framerate set to: " << FRAMERATE << endl;
        cout << "Model count: " << bufferCount << endl;
    }

    PrintLog("Initializing module and window");
    // Initialize modules and setup Window
    GLFWwindow* window = initializeAndCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sunset");
    if (!window)
    {
        cerr << "Error when setting up window" << endl;
        return -1;
    }

    PrintLog("Initializing shaders");
    // Shaders
    Shader shader = Shader("shaders/base/flatShader.vs", 
        DebugActive(DEBUG_DRAW_NORMS) ? "shaders/base/normShader.fs" : "shaders/base/flatShader.fs");
    if (!shader.isValid) {
        cerr << "Error when compiling base shader" << endl;
        return -1; }

    Shader sunShader = Shader("shaders/sun/sunShader.vs", "shaders/sun/sunShader.fs");
    if (!sunShader.isValid) {
        cerr << "Error when compiling sun shader" << endl;
        return -1; }

    Shader occlusionShader = Shader("shaders/occlusion/occlusionShader.vs", "shaders/occlusion/occlusionShader.fs");
    if (!occlusionShader.isValid) {
        cerr << "Error when compiling sun shader" << endl;
        return -1; }

    Shader depthShader = Shader("shaders/depth/depthShader.vs", "shaders/depth/depthShader.fs");
    if (!depthShader.isValid) {
        cerr << "Error when compiling depth shader" << endl;
        return -1; }

    // TODO: How is buffer count generated?
    unsigned int VBOs[bufferCount], EBOs[bufferCount];
    unsigned int depthFBO, depthMap, occlusionFBO, occlusionMap;

    PrintLog("Generating buffers");
    genBuffers(bufferCount, VBOs, VAOs, EBOs, depthFBO, depthMap, occlusionFBO, occlusionMap);

    PrintLog("Generating models");

    // Generic scene info
    sunDirection = normalize(vec3(3, 0.01, 4));
    vec3 lightDirection = -sunDirection;
    sunTransform = mat4(1.0f);
    vec3 sunPosition = sunDirection * sunRenderDist;
    sunTransform = translate(sunTransform, sunPosition);
    sunTransform *= inverse(lookAt(lightDirection, sunDirection, vec3(0, 1, 0)));

    // Individual models
    for (int i = 0; i < bufferCount; i++)
    {
        // TODO: Do I need to be worred about gc?
        vector<float> vertices;
        vector<int> indices;
        int triangleCount;
        ModelData modelData;

        if (!createModel(i, vertices, indices, modelData, triangleCount))
        {
            cerr << "Error when generating model " << i << endl;
            return -1;
        }

        triangleCounts.push_back(triangleCount);
        modelDatas.push_back(modelData);
        bindBuffer(i, VBOs, VAOs, EBOs, vertices, indices);
    }

    PrintLog("Creating camera");
    // Create camera
    if (DebugActive(DEBUG_FREEHAND_CAMERA))
    {
        camera = new FreeCamera(vec3(-2, 0.5f, 0), 0, 0);
    }
    else
    {
        camera = new FixedCamera(vec3(0, 2, 3));
    }

    // TODO: Calculate close and far planes based on model and sun
    float nearPlane = 1.0f, farPlane = 10.0f;

    // Light view matrix (currently, this doesn't change)
    mat4 lightProjection, lightView, lightViewMatrix;
    lightProjection = ortho(-1.0f, 1.0f, -1.0f, 1.0f, nearPlane, farPlane);
    lightView = lookAt(sunDirection * sunShadowDist, vec3(0.0f), vec3(0.0, 1.0, 0.0));
    lightViewMatrix = lightProjection * lightView;

    // Set settings
    glEnable(GL_DEPTH_TEST);  
    glEnable(GL_CULL_FACE);  
    glCullFace(GL_BACK);

    // Framerate calculations
    float accumTime = 0.0f;
    float deltaTime = 0.0f;
    float prevFrame = static_cast<float>(glfwGetTime());
    bool shouldRender = true;

    PrintLog("Beginning Render Loop");
    // Render loop
    while(!glfwWindowShouldClose(window))
    {
        // Render
        float currentTime = static_cast<float>(glfwGetTime());
        accumTime += currentTime - prevFrame;
        prevFrame = currentTime;

        if (accumTime > PER_FRAME)
        {
            deltaTime = accumTime - fmod(accumTime, PER_FRAME);
            accumTime = fmod(accumTime, PER_FRAME);

            if (DebugActive(DEBUG_FRAMERATE))
            {
                // TODO: Debug actual framerate
                if (deltaTime > PER_FRAME)
                {
                    cout << "Skipped " << deltaTime / PER_FRAME - 1 << " frame(s)" << endl;
                }
            }

            shouldRender = true;
        }

        if (shouldRender)
        {
            shouldRender = false;

            // Clear frame

            // Process inputs
            processInput(window);
            camera->ProcessInput(window, deltaTime);

            // Depth buffer render
            glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
            glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            
            depthShader.setActive();
            depthShader.setUniform("lightView", lightViewMatrix);
            
            // TODO: Peter panning?
            // No need to render sun for shadowmap
            render(depthShader, 0, MODEL_DEFAULT);

            // Projection and view
            mat4 projection = perspective(radians(45.0f), ASPECT_RATIO, 0.1f, 100.0f );
            mat4 view = camera->GetLookAt();
            vec4 sunScreenPrePos = projection * view * vec4(sunPosition, 1.0);
            vec3 sunScreenPos = vec3(sunScreenPrePos) / sunScreenPrePos.w;
            sunScreenPos = sunScreenPos * 0.5f + 0.5f;

            // Bind occlusion buffer
            glViewport(0, 0, curWidth, curHeight);
            glBindFramebuffer(GL_FRAMEBUFFER, occlusionFBO);
            glClearColor(COLOR_BLACK[0], COLOR_BLACK[1], COLOR_BLACK[2], COLOR_BLACK[3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            occlusionShader.setActive();
            occlusionShader.setUniform("view", view);
            occlusionShader.setUniform("projection", projection);

            render(occlusionShader, RENDER_LIGHTOCCLUSION);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, depthMap);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, occlusionMap);
                    
            //Bind main texture
            glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
            glViewport(0, 0, curWidth, curHeight);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Main render
            shader.setActive();
            shader.setUniform("shadowMap", 0);
            shader.setUniform("occlusionMap", 1);
            shader.setUniform("lightView", lightViewMatrix);
            shader.setUniform("lightDir", lightDirection);
            shader.setUniform("lightScreenPos", sunScreenPos);
            
            // Projection
            shader.setUniform("projection", projection);
            
            // View
            shader.setUniform("view", view);

            render(shader, RENDER_NORM | RENDER_COLOR, MODEL_DEFAULT);

            // Sun render
            sunShader.setActive();
            sunShader.setUniform("projection", projection);
            sunShader.setUniform("view", view);
            render(sunShader, RENDER_COLOR, MODEL_LIGHTSOURCE);

            // Swap buffers and poll events
            glfwSwapBuffers(window);
            glfwPollEvents(); 
        }   
    }

    // Deallocate
    glDeleteVertexArrays(bufferCount, VAOs);
    glDeleteBuffers(bufferCount, VBOs);
    glDeleteFramebuffers(1, &depthFBO);
    shader.deleteProgram();

    glfwTerminate();
    return 0;
}

void render(Shader shader, unsigned int renderflags, unsigned int drawflags)
{
    
    for (int i = 0; i < bufferCount; i++)
    {
        ModelData modelData = modelDatas[i];
        
        // Skip models not in drawflags
        if (!(drawflags & modelData.modelType))
            continue;

        glBindVertexArray(VAOs[i]);

        mat4 model = mat4(1.0f);
        if (modelData.modelType & MODEL_LIGHTSOURCE)
        {   // Transform lightsource to be at sun position
            model = sunTransform;
        }
        else if (modelData.modelType & MODEL_DEFAULT)
        {   // Use modeldata transform
            model = translate(model, modelData.translation);
            vec3 rotationAxis = vec3(modelData.rotation);
            if (rotationAxis != vec3(0))
                model = rotate(model, modelData.rotation.w, rotationAxis);
        }

        shader.setUniform("model", model);

        if (renderflags & RENDER_NORM)
        {
            mat3 normMat = mat3(transpose(inverse(model)));
            shader.setUniform("normMatrix", normMat);
        }

        // Color? Texture or flat
        if (renderflags & RENDER_COLOR)
        {
            shader.setUniform("baseColor", modelData.color);
            shader.setUniform("lightColor", vec3(1.0f, 1.0f, 1.0f));
        }

        // Set isLight uniform during light occlusion
        if (renderflags & RENDER_LIGHTOCCLUSION)
        {
            if (modelData.modelType & MODEL_LIGHTSOURCE)
                shader.setUniform("isLight", true);
            else
                shader.setUniform("isLight", false);
        }

        // Get triangle count based on model
        glDrawElements(GL_TRIANGLES, 3 * triangleCounts[i], 
                GL_UNSIGNED_INT, 0);
    }

}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void mouseCallback(GLFWwindow* window, double xPosD, double yPosD)
{
    (void) window; 
    float xPos = static_cast<float>(xPosD);
    float yPos = static_cast<float>(yPosD);

    if (!mouseSet)
    {
        mouseX = xPos;
        mouseY = yPos;
        mouseSet = true;
    }

    float xChange = xPos - mouseX;
    float yChange = mouseY - yPos;

    mouseX = xPos;
    mouseY = yPos;

    camera->ProcessMouse(xChange, yChange);
}

GLFWwindow* initializeAndCreateWindow(int screenWidth, int screenHeight, const char* windowName)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
    // Setup Window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, windowName, NULL, NULL);
    if (window == NULL)
    {
        cerr << "Error when initializing window" << endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cerr << "Error when initializing GLAD" << endl;
        return nullptr;
    }

    return window;
}

unsigned int genBuffers(int bufferCount, unsigned int (&VBOs)[], unsigned int (&VAOs)[], unsigned int (&EBOs)[],
    unsigned int &depthFBO, unsigned int &depthMap, unsigned int& occlusionFBO, unsigned int& occlusionMap)
{
    glGenBuffers(bufferCount, VBOs);  
    glGenVertexArrays(bufferCount, VAOs);
    glGenBuffers(bufferCount, EBOs);

    glGenFramebuffers(1, &depthFBO);
    glGenTextures(1, &depthMap);
    
    // Depthmap texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                SHADOW_RESOLUTION, SHADOW_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); 
    // Assume out of range is out of shadow
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, COLOR_WHITE); 

    // Binding texture to buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

    //Occlusion texture
    glGenFramebuffers(1, &occlusionFBO);
    glGenTextures(1, &occlusionMap);

    // Depthmap texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, occlusionMap);

        // TODO: WHAT TO DO ABOUT RESIZING?
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
                SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Binding texture to buffer
    glBindFramebuffer(GL_FRAMEBUFFER, occlusionFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, occlusionMap, 0);

    // Occlusion buffer requires depth
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);  
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  



    return 1;
}

unsigned int bindBuffer(int bufferIndex, unsigned int (&VBOs)[], unsigned int (&VAOs)[], 
        unsigned int (&EBOs)[], vector<float> vertices, vector<int> indices)
{
    glBindVertexArray(VAOs[bufferIndex]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[bufferIndex]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[bufferIndex]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);  
    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

    // Position data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture / UV data
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindVertexArray(0);

    return 1;
}

void framebufferSizeCallback(GLFWwindow* window, int newWidth, int newHeight)
{
    // Resize
    curWidth = newWidth;
    curHeight = newHeight;
    glViewport(0, 0, curWidth, curHeight);
    
    // Unused variables
    (void) window;
}

unsigned int handleArgs(int argc, char*argv[])
{
    int c;
    while ((c = getopt(argc, argv, "nfchv")) != -1) {
        switch (c) {
            case 'n': 
                SetDebug(DEBUG_DRAW_NORMS);
                break;
            case 'f':
                SetDebug(DEBUG_FRAMERATE); 
                break;
            case 'c':
                SetDebug(DEBUG_FREEHAND_CAMERA);
                break;
            case 'v':
                SetDebug(DEBUG_VERBOSE);
                break;
            // TODO: Command line argument to set framerate
            case 'h':
                displayHelp();
                return 0;
            default:
                cerr << "Unrecognized command line argument" << endl;
                displayHelp();
                return 0;
        }
    }
    return 1;
}

void displayHelp()
{
    cout << "Usage: ./sunset.exe [options]" << endl;
    cout << "   options:" << endl;
    cout << "      -v display verbose logs" << endl;
    cout << "      -n draw normals" << endl;
    cout << "      -f enable framerate debug" << endl;
    cout << "      -c enable full camera control" << endl;
}