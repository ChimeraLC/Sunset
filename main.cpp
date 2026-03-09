#include <stdlib.h>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <unistd.h>

#include <iostream>
#include <cmath>
#include <string>
#include <vector>

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

// Debug Settings (TODO)

// Config settings
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;
int curWidth = SCREEN_WIDTH;
int curHeight = SCREEN_HEIGHT;
const float ASPECT_RATIO = (float) SCREEN_WIDTH / (float) SCREEN_HEIGHT;
const float FRAMERATE = 30.0f;
const float PER_FRAME = 1 / FRAMERATE;

const int SHADOW_RESOLUTION = 512;

const float COLOR_WHITE[] = {1.0, 1.0, 1.0, 1.0};

// Global values
const int bufferCount = 2;
unsigned int VAOs[bufferCount];
vector<int> triangleCounts;

int main(int argc, char *argv[])
{
    if (!handleArgs(argc, argv))
    {
        return -1;
    }

    // Initialize modules and setup Window
    GLFWwindow* window = initializeAndCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sunset");
    if (!window)
    {
        cerr << "Error when setting up window" << endl;
        return -1;
    }

    // Shaders
    Shader shader = Shader("shaders/flatShader.vs", 
        DebugActive(DEBUG_DRAW_NORMS) ? "shaders/normShader.fs" : "shaders/flatShader.fs");
    if (!shader.isValid)
    {
        cerr << "Error when compiling base shader" << endl;
        return -1;
    }

    Shader depthShader = Shader("shaders/depthShader.vs", "shaders/depthShader.fs");
    if (!depthShader.isValid)
    {
        cerr << "Error when compiling depth shader" << endl;
        return -1;
    }

    // TODO: How is buffer count generated?
    unsigned int VBOs[bufferCount], EBOs[bufferCount];
    unsigned int depthFBO, depthMap;

    genBuffers(bufferCount, VBOs, VAOs, EBOs, depthFBO, depthMap);

    // Generate models TODO: Currently just one
    for (int i = 0; i < bufferCount; i++)
    {
        // TODO: Do I need to be worred about gc?
        vector<float> vertices;
        vector<int> indices;
        int triangleCount;

        if (!createModel(i, vertices, indices, triangleCount))
        {
            cerr << "Error when generating model " << i << endl;
            return -1;
        }

        triangleCounts.push_back(triangleCount);
        bindBuffer(i, VBOs, VAOs, EBOs, vertices, indices);
    }

    // Create camera
    PositionalCamera camera = PositionalCamera(vec3(0, 2, 3));
    camera.SetTarget(vec3(0, 0, 0));
    float cameraTime = 0;

    vec3 sunPosition = vec3(3, 3, 4);
    vec3 lightDirection = -sunPosition;

    // TODO: Calculate close and far planes based on model and sun
    float nearPlane = 1.0f, farPlane = 10.0f;

    // Light view matrix (currently, this doesn't change)
    mat4 lightProjection, lightView, lightViewMatrix;
    lightProjection = ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
    lightView = lookAt(sunPosition, vec3(0.0f), vec3(0.0, 1.0, 0.0));
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
            glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

            // Process inputs
            processInput(window);

            // Depth buffer render
            depthShader.setActive();
            depthShader.setUniform("lightView", lightViewMatrix);
            glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
            glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            
            // TODO: Peter panning?
            render(depthShader, false, false);

            // Main render
            shader.setActive();
            shader.setUniform("shadowMap", 0);
            shader.setUniform("lightView", lightViewMatrix);
            shader.setUniform("lightDir", lightDirection);
            
            // Projection
            mat4 projection = perspective(radians(45.0f), ASPECT_RATIO, 0.1f, 100.0f );
            shader.setUniform("projection", projection);
            
            // View
            cameraTime += deltaTime / 3;
            camera.SetPosition(vec3(3 * cos(cameraTime), 2, 3 * sin(cameraTime)));
            mat4 view = camera.GetLookAt();
            shader.setUniform("view", view);

            glViewport(0, 0, curWidth, curHeight);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            render(shader, true, true);

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

void render(Shader shader, bool usesNorm, bool usesColor)
{
    
    for (int i = 0; i < bufferCount; i++)
    {

        glBindVertexArray(VAOs[i]);

        // TODO: Get model transform based on model
        mat4 model = mat4(1.0f);
        shader.setUniform("model", model);

        if (usesNorm)
        {
            mat3 normMat = mat3(transpose(inverse(model)));
            shader.setUniform("normMatrix", normMat);
        }

        // Color? Texture or flat
        if (usesColor)
        {
            shader.setUniform("baseColor", vec3(0.6f * i, 0.3f, 0.0f));
            shader.setUniform("lightColor", vec3(1.0f, 1.0f, 1.0f));
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
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cerr << "Error when initializing GLAD" << endl;
        return nullptr;
    }

    return window;
}

unsigned int genBuffers(int bufferCount, unsigned int (&VBOs)[], unsigned int (&VAOs)[], unsigned int (&EBOs)[],
    unsigned int &depthFBO, unsigned int &depthMap)
{
    glGenBuffers(bufferCount, VBOs);  
    glGenVertexArrays(bufferCount, VAOs);
    glGenBuffers(bufferCount, EBOs);

    glGenFramebuffers(1, &depthFBO);
    glGenTextures(1, &depthMap);
    
    // Depthmap texture
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
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    return 1;
}

unsigned int bindBuffer(int bufferIndex, unsigned int (&VBOs)[], unsigned int (&VAOs)[], 
        unsigned int (&EBOs)[], vector<float> vertices, vector<int> indices)
{
    glBindVertexArray(VAOs[bufferIndex]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[bufferIndex]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[bufferIndex]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices), &vertices[0], GL_STATIC_DRAW);  
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices), &indices[0], GL_STATIC_DRAW);

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
    while ((c = getopt(argc, argv, "nfh")) != -1) {
        switch (c) {
            case 'n': 
                SetDebug(DEBUG_DRAW_NORMS);
                break;
            case 'f':
                SetDebug(DEBUG_FRAMERATE); 
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
    cout << "      -n draw normals" << endl;
    cout << "      -f enable framerate debug" << endl;
}