#include <stdlib.h>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "vertexdata.h"
#include "shader.h"
#include "camera.h"

#include "main.h"

using namespace std;
using namespace glm;

// Config settings
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;
const float ASPECT_RATIO = (float) SCREEN_WIDTH / (float) SCREEN_HEIGHT;
const float FRAMERATE = 30.0f;
const float PER_FRAME = 1 / FRAMERATE;

int main()
{
    // Initialize modules and setup Window
    GLFWwindow* window = initializeAndCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sunset");
    if (!window)
    {
        std::cerr << "Error when setting up window" << std::endl;
        return -1;
    }

    // Shaders
    Shader shader = Shader("shaders/flatShader.vs", "shaders/flatShader.fs");
    if (!shader.isValid)
    {
        std::cerr << "Error when compiling shaders" << std::endl;
        return -1;
    }

    // Vertex buffers
    unsigned int VBO, VAO, EBO;
    linkBuffers(VBO, VAO, EBO);

    // Create camera
    PositionalCamera camera = PositionalCamera(vec3(0, 0, 3));
    camera.SetTarget(vec3(0, 0, 0));
    float cameraTime = 0;

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

            shouldRender = true;
        }

        if (shouldRender)
        {
            shouldRender = false;

            // Clear frame
            glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
            glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            // Process inputs
            processInput(window);

            // Draw
            shader.setActive();

            // Projection
            mat4 projection = perspective(radians(45.0f), ASPECT_RATIO, 0.1f, 100.0f );
            shader.setUniform("projection", projection);
            
            // View
            cameraTime += deltaTime;
            camera.SetPosition(vec3(3 * cos(cameraTime), 0, 3 * sin(cameraTime)));
            mat4 view = camera.GetLookAt();
            shader.setUniform("view", view);

            glBindVertexArray(VAO);

            mat4 model = mat4(1.0f);
            model = rotate(model, radians(20.0f), vec3(1.0f, 0.3f, 0.5f));
            shader.setUniform("model", model);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // Swap buffers and poll events
            glfwSwapBuffers(window);
            glfwPollEvents(); 
        }   
    }

    // Deallocate
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    shader.deleteProgram();

    glfwTerminate();
    return 0;
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

// TODO: vertices input in main, and not hardcoded
unsigned int linkBuffers(unsigned int& VBO, unsigned int& VAO, unsigned int& EBO)
{
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);    
    
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // TODO: Formalize this somehow
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return 1;
}

void framebufferSizeCallback(GLFWwindow* window, int newWidth, int newHeight)
{
    // Resize
    glViewport(0, 0, newWidth, newHeight);
    
    // Unused variables
    (void) window;
}