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
#include "texture.h"
#include "consts.h"

#include "main.h"

using namespace std;
using namespace glm;

// Config settings
int curWidth = SCREEN_WIDTH;
int curHeight = SCREEN_HEIGHT;
const float ASPECT_RATIO = (float) SCREEN_WIDTH / (float) SCREEN_HEIGHT;
const float FRAMERATE = 40.0f;
const float PER_FRAME = 1 / FRAMERATE;

const int SHADOW_RESOLUTION = 4096;
const int SHADOW_RT_DOWNSCALE = 4;

const float COLOR_WHITE[] = {1.0, 1.0, 1.0, 1.0};
const float COLOR_BLACK[] = {0.0, 0.0, 0.0, 1.0};
vec3 sunDirection = vec3(0, 1, 0);
mat4 sunTransform = mat4(1.0);
float sunShadowDist = 4;    // Distance shadowmap is rendered from
float sunRenderDist = 15;   // Distance sun model is rendered

vec3 sunColor;
vec3 lightColor;

// Global values
const int bufferCount = MODEL_COUNT;
unsigned int VAOs[bufferCount], FBOs[FRAMEBUFFER_COUNT], FTexs[FRAMEBUFFER_COUNT];
unsigned int quadVAO;
vector<int> triangleCounts;
vector<ModelData> modelDatas;

unsigned int skyTexture, cloudTexture;
Image currentImage;
const int designImageCount = 2;
// First texture decides clouds/stars and tree. second texture decides grass and day/night
Image designImages[designImageCount];
bool inDesignStage = true;
int currentDesignStage = 0;
const unsigned int DESIGN_EXIT_KEY = GLFW_KEY_R;
bool exitKeyHeld = false;

// TODO: Really need to cut down on shader count somehow, or, at least move into enum
Shader flatShader, postShader, occlusionShader, depthShader, screenShader, radialShader, 
    bloomShader, skyboxShader, skyShader, grassShader, depthFoliageShader, screenPosShader;

Camera* camera;
float mouseX;
float mouseY;
bool mouseSet;
bool mouseJustPressed;
float mouseXPrev;
float mouseYPrev;
float worldTime = 0;

mat4 lightViewMatrix;

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
    if (!compileShaders())
        return -1;

    // TODO: How is buffer count generated?
    unsigned int VBOs[bufferCount], EBOs[bufferCount], quadVBO;

    PrintLog("Generating buffers");
    genBuffers(bufferCount, VBOs, VAOs, EBOs, quadVBO);

    // Textures that are drawn to
    unsigned int designTexture;
    glGenTextures(1, &designTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, designTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    for (int i = 0; i < designImageCount; i++)
        designImages[i] = generateEmpty(SCREEN_WIDTH, SCREEN_HEIGHT);
  
    // Skipping design stage
    if (DebugActive(DEBUG_SKIP_DESIGN) || designImageCount <= 0) 
    {
        inDesignStage = false;
    }

    // // MARK: Design stage
    PrintLog("Entering design stage");
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glfwSetCursorPos(window, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    while(!glfwWindowShouldClose(window))
    {
        processInput(window);
        if (!inDesignStage)
            break;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        currentImage = designImages[currentDesignStage];
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, currentImage.width, currentImage.height,
            0, GL_RGBA, GL_UNSIGNED_BYTE, currentImage.data);
        glBindTexture(GL_TEXTURE_2D, designTexture);

        // Draw texture to screen
        screenShader.setActive();
        screenShader.setUniform("screenTex", 0);
        renderScreenQuad(screenShader);

        // Drawing mouse
        screenPosShader.setActive();
        screenPosShader.setUniform("baseColor", vec3(0, 0, 0));
        screenPosShader.setUniform("offset", vec2(mouseX, 
            SCREEN_HEIGHT-mouseY) / vec2(SCREEN_WIDTH, SCREEN_HEIGHT) * 2.f - 1.f);
        
        renderScreenQuad(screenPosShader);
        
        glfwSwapBuffers(window);
        glfwPollEvents(); 
    }

    PrintLog("Generating models");

    // Generic scene info
    sunDirection = normalize(vec3(3, 2, 4));
    vec3 lightDirection = -sunDirection;
    sunTransform = mat4(1.0f);
    // Sun is visually lower than the actual lightsource TODO: Fix sun position
    vec3 sunPosition = sunDirection * sunRenderDist;
    sunPosition.y = 1.f;
    sunTransform = translate(sunTransform, sunPosition);
    sunTransform *= inverse(lookAt(lightDirection, sunDirection, WORLD_UP));

    float windSpeed = 100 + randFloat() * 200;

    // Individual models
    if (!genModels(VBOs, EBOs))
        return -1;

    // The rest of this depends on design input
    PrintLog("Generating textures");
    genTextures();

    lightColor = getTime() == NIGHT ? vec3(0.9f, 0.9f, 0.9f) : vec3(1.0f, 0.35f, 0.4f);
    sunColor = getTime() == NIGHT ? vec3(0.9f, 0.75f, 0.78f) : vec3(0.9f, 0.7f, 0.7f);

    PrintLog("Creating camera");
    // Create camera
    if (DebugActive(DEBUG_FREEHAND_CAMERA))
    {
        FreeCamera* freeCamera = new FreeCamera(vec3(-2, 0.2f, 0), 0, 0);
        freeCamera->SetXBound(vec2(-2, 2));
        freeCamera->SetZBound(vec2(-2, 2));
        freeCamera->SetYBound(vec2(0.1, 2));
        camera = freeCamera;
    }
    else
    {
        camera = new FixedCamera(0.2, 2, 2.5);
    }

    // TODO: Calculate close and far planes based on model and sun
    float nearPlane = 1.f, farPlane = 10.0f;

    // Light view matrix (currently, this doesn't change)
    mat4 lightProjection, lightView;
    lightProjection = ortho(-3.0f, 3.0f, -1.0f, 1.0f, nearPlane, farPlane);
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

    // Precalculate maps that don't change (shadowmap)
    precalc();

    PrintLog("Beginning Render Loop");
    // MARK: Render loop
    while(!glfwWindowShouldClose(window))
    {
        // Render
        float currentTime = static_cast<float>(glfwGetTime());
        accumTime += currentTime - prevFrame;
        prevFrame = currentTime;

        // Check framerate
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
            worldTime += deltaTime;
        }

        if (shouldRender)
        {
            shouldRender = false;
            // Process inputs
            processInput(window);
            camera->ProcessInput(window, deltaTime);

            // TODO: Low res foliage
            glViewport(0, 0, SHADOW_RESOLUTION / SHADOW_RT_DOWNSCALE, 
                SHADOW_RESOLUTION / SHADOW_RT_DOWNSCALE);
            bindFramebuffer(DEPTH_MAP_RT);
            glClear(GL_DEPTH_BUFFER_BIT);

            depthFoliageShader.setActive();
            depthFoliageShader.setUniform("lightView", lightViewMatrix);
            render(depthFoliageShader, INSTANCED | TIME_DEPENDENT | CULL_DISABLED, MODEL_FOLIAGE);
        
            bindTexture(DEPTH_MAP_RT);

            // Projection and view
            mat4 projection = perspective(radians(45.0f), ASPECT_RATIO, 0.1f, 100.0f );
            mat4 view = camera->GetLookAt();
            mat4 viewNoPosition = glm::mat4(glm::mat3(view));

            vec4 sunScreenPrePos = projection * viewNoPosition * vec4(sunPosition, 1.0);
            vec3 sunScreenPos = vec3(sunScreenPrePos) / sunScreenPrePos.w;
            sunScreenPos = sunScreenPos * 0.5f + 0.5f;

            // Bind occlusion buffer
            glViewport(0, 0, curWidth, curHeight);
            bindFramebuffer(OCCLUSION_MAP);
            glClearColor(COLOR_BLACK[0], COLOR_BLACK[1], COLOR_BLACK[2], COLOR_BLACK[3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Rendering regular models
            occlusionShader.setActive();
            occlusionShader.setUniform("view", view);
            occlusionShader.setUniform("projection", projection);

            render(occlusionShader, RENDER_LIGHTOCCLUSION, MODEL_DEFAULT);

            occlusionShader.setUniform("view", viewNoPosition);
            render(occlusionShader, RENDER_LIGHTOCCLUSION, MODEL_LIGHTSOURCE);

            grassShader.setActive();
            grassShader.setUniform("view", view);
            grassShader.setUniform("projection", projection);
            grassShader.setUniform("renderOcclusion", true);

            render(grassShader, CULL_DISABLED | INSTANCED |
                RENDER_LIGHTOCCLUSION | RENDER_COLOR, MODEL_FOLIAGE);

            // Rendering skybox
            glActiveTexture(GL_TEXTURE0 + FRAMEBUFFER_COUNT); // TODO: non-fb tex enum
            glBindTexture(GL_TEXTURE_CUBE_MAP, skyTexture);
            skyboxShader.setActive();
            skyboxShader.setUniform("skyboxTex", FRAMEBUFFER_COUNT);
            skyboxShader.setUniform("projection", projection);
            skyboxShader.setUniform("view", viewNoPosition);
            skyboxShader.setUniform("occlusionRendering", true);
            render(skyboxShader, 0, MODEL_SKYBOX);
            bindTexture(OCCLUSION_MAP);

            // Lightrays map
            bindFramebuffer(LIGHTRAYS_MAP);
            glClearColor(COLOR_BLACK[0], COLOR_BLACK[1], COLOR_BLACK[2], COLOR_BLACK[3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            radialShader.setActive();
            radialShader.setUniform("screenTex", OCCLUSION_MAP);
            radialShader.setUniform("lightScreenPos", sunScreenPos);
            renderScreenQuad(radialShader);
                    
            bindTexture(LIGHTRAYS_MAP);
            
            if (DebugActive(DEBUG_DRAW_LIGHTRAYS))
            {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // Screen shader
                screenShader.setActive();
                screenShader.setUniform("screenTex", LIGHTRAYS_MAP);
                renderScreenQuad(screenShader);
            }
            else
            {
                // Bloom postprocessing (two pass)
                bindFramebuffer(TEMPORARY_A);
                glClear(GL_COLOR_BUFFER_BIT);
                bloomShader.setActive();
                bloomShader.setUniform("occlusionTex", OCCLUSION_MAP);
                bloomShader.setUniform("screenTex", OCCLUSION_MAP);
                bloomShader.setUniform("stage", 0);
                renderScreenQuad(bloomShader);
                bindTexture(TEMPORARY_A);

                // Reusing occlusion map since it's not used afterwards
                bindFramebuffer(OCCLUSION_MAP);
                glClear(GL_COLOR_BUFFER_BIT);
                bloomShader.setUniform("screenTex", TEMPORARY_A);
                bloomShader.setUniform("stage", 1);
                renderScreenQuad(bloomShader);
                bindTexture(OCCLUSION_MAP);

                //Bind main texture
                glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
                glViewport(0, 0, curWidth, curHeight);

                bindFramebuffer(POSTPROCESS);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glDepthMask(GL_FALSE); // TODO: Render last?
                skyboxShader.setActive();
                skyboxShader.setUniform("lightraysTex", LIGHTRAYS_MAP);
                skyboxShader.setUniform("occlusionRendering", false);
                render(skyboxShader, RENDER_COLOR, MODEL_SKYBOX);

                glEnable(GL_BLEND); // Clouds are only transparent thing
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
                glActiveTexture(GL_TEXTURE1 + FRAMEBUFFER_COUNT); // TODO: non-fb tex enum
                glBindTexture(GL_TEXTURE_2D, cloudTexture);
                skyShader.setActive();
                skyShader.setUniform("wind", windSpeed);
                skyShader.setUniform("skyTex", FRAMEBUFFER_COUNT + 1);
                skyShader.setUniform("lightraysTex", LIGHTRAYS_MAP);
                skyShader.setUniform("projection", projection);
                skyShader.setUniform("view", viewNoPosition);
                render(skyShader, TIME_DEPENDENT | RENDER_COLOR, MODEL_SKY);
                glDepthMask(GL_TRUE);
                glDisable(GL_BLEND);

                //Main render
                flatShader.setActive();
                flatShader.setUniform("camPos", camera->GetPosition());
                flatShader.setUniform("shadowMap", DEPTH_MAP_PRE);
                flatShader.setUniform("shadowMapRT", DEPTH_MAP_RT);
                flatShader.setUniform("lightraysTex", LIGHTRAYS_MAP);
                flatShader.setUniform("lightView", lightViewMatrix);
                flatShader.setUniform("lightDir", lightDirection);
                
                // Projection / view
                flatShader.setUniform("projection", projection);
                flatShader.setUniform("view", view);

                render(flatShader, RENDER_NORM | RENDER_COLOR, MODEL_DEFAULT);

                // Folliage
                grassShader.setActive();
                grassShader.setUniform("shadowMap", DEPTH_MAP_PRE);
                grassShader.setUniform("shadowMapRT", DEPTH_MAP_RT);
                grassShader.setUniform("lightraysTex", LIGHTRAYS_MAP);
                grassShader.setUniform("lightView", lightViewMatrix);
                grassShader.setUniform("lightDir", lightDirection);
                grassShader.setUniform("renderOcclusion", false);
            
                render(grassShader, TIME_DEPENDENT | CULL_DISABLED | INSTANCED |
                    RENDER_COLOR | RENDER_NORM, MODEL_FOLIAGE);

                bindTexture(POSTPROCESS);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    postShader.setActive();
                    postShader.setUniform("screenTex", POSTPROCESS);
                    postShader.setUniform("bloomTex", OCCLUSION_MAP);
                    postShader.setUniform("sunColor", sunColor);
                    renderScreenQuad(postShader);
            }
            
            // Swap buffers and poll events
            glfwSwapBuffers(window);
            glfwPollEvents(); 
        }   
    }

    // Deallocate
    glDeleteBuffers(bufferCount, VBOs);
    glDeleteVertexArrays(bufferCount, VAOs);
    glDeleteBuffers(bufferCount, VBOs);
    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteFramebuffers(FRAMEBUFFER_COUNT, FBOs);
    glDeleteTextures(FRAMEBUFFER_COUNT, FTexs);

    // Calling this deletes all shaders
    flatShader.deleteProgram();

    glfwTerminate();
    return 0;
}

// MARK: Render
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
        else if (modelData.modelType & MODEL_DEFAULT
        || modelData.modelType & MODEL_FOLIAGE)
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
            shader.setUniform("lightColor", lightColor);
        }

        if (renderflags & CULL_DISABLED)
            glDisable(GL_CULL_FACE);

        // Set isLight uniform during light occlusion
        if (renderflags & RENDER_LIGHTOCCLUSION)
        {
            if (modelData.modelType & MODEL_LIGHTSOURCE)
                shader.setUniform("isLight", true);
            else
                shader.setUniform("isLight", false);
        }

        // Time value
        if (renderflags & TIME_DEPENDENT)
        {
            shader.setUniform("time", worldTime);
        }

        // Get triangle count based on model
        if (renderflags & INSTANCED)
        {
            glDrawElementsInstanced(GL_TRIANGLES, 3 * triangleCounts[i], 
                    GL_UNSIGNED_INT, 0, modelData.instanceCount);
        }
        else
        {
            glDrawElements(GL_TRIANGLES, 3 * triangleCounts[i], 
                    GL_UNSIGNED_INT, 0);
        }

        if (renderflags & CULL_DISABLED)
            glEnable(GL_CULL_FACE);

    }
}

void renderScreenQuad(Shader shader)
{
    (void) shader;
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void bindFramebuffer(TextureBuffer buffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBOs[buffer]);
}
void bindTexture(TextureBuffer buffer)
{
        glActiveTexture(GL_TEXTURE0 + buffer);
        glBindTexture(GL_TEXTURE_2D, FTexs[buffer]);
}

void processInput(GLFWwindow* window)
{
    if (inDesignStage)
    {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
        {
            int brushSize = 35;

            float mouseStroke = 0;
            vec2 strokeDir;
            if (mouseJustPressed) // Catching frameskips
            {
                mouseStroke = length(vec2(mouseX - mouseXPrev, mouseY - mouseYPrev));
                strokeDir = vec2(mouseXPrev - mouseX, mouseYPrev - mouseY) / mouseStroke;
            }

            //if (mouseX >= 0 && mouseX < SCREEN_WIDTH && mouseY >= 0 && mouseY < SCREEN_HEIGHT)            

            for (int step = 0; step <= mouseStroke; step+= brushSize / 2)
            {
                float posX = mouseX + strokeDir.x * step;
                float posY = mouseY + strokeDir.y * step;
                for (int i = -brushSize; i < brushSize; i++)
                {
                    for (int j = -brushSize; j < brushSize; j++)
                        setColor(currentImage, SCREEN_HEIGHT - posY + j, posX + i, 0, 0, 0);
                }
            }

            mouseJustPressed = true;
        }
        else
        {
            mouseJustPressed = false;
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            if (!exitKeyHeld)
            {
                currentDesignStage++;
                if (currentDesignStage >= designImageCount)
                { 
                    PrintLog("Exited design stage");
                    inDesignStage = false;
                }
                glfwSetCursorPos(window, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
                exitKeyHeld = true;
            }
        }
        else
        {
            exitKeyHeld = false;
        }
    }
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

    mouseXPrev = mouseX;
    mouseYPrev = mouseY;
    mouseX = xPos;
    mouseY = yPos;

    if (inDesignStage)
    {
    }
    else
    {
        camera->ProcessMouse(xChange, yChange);
    }
}

// MARK: Init
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

// TODO: Clean this up TT
bool compileShaders()
{
    flatShader = Shader("shaders/base/flatShader.vs", 
        DebugActive(DEBUG_DRAW_NORMS) ? "shaders/base/normShader.fs" : "shaders/base/flatShader.fs", "base shader");
    postShader = Shader("shaders/postprocess/screenShader.vs", "shaders/postprocess/postShader.fs", "postprocess shader");
    occlusionShader = Shader("shaders/occlusion/occlusionShader.vs", "shaders/occlusion/occlusionShader.fs", "occlusion shader");
    depthShader = Shader("shaders/depth/depthShader.vs", "shaders/depth/depthShader.fs", "depth shader");
    depthFoliageShader = Shader("shaders/depth/depthFoliageShader.vs", "shaders/depth/depthShader.fs", "depth foliage shader");
    screenShader = Shader("shaders/postprocess/screenShader.vs", "shaders/postprocess/screenShader.fs", "screen shader");
    radialShader = Shader("shaders/postprocess/screenShader.vs", "shaders/postprocess/radialBlurShader.fs", "radial blur shader");
    bloomShader = Shader("shaders/postprocess/screenShader.vs", "shaders/postprocess/bloomShader.fs", "bloom shader");
    skyboxShader = Shader("shaders/horizon/skyboxShader.vs", "shaders/horizon/skyboxShader.fs", "skybox shader");
    grassShader = Shader("shaders/foliage/grassShader.vs", "shaders/foliage/grassShader.fs", "grass shader");
    screenPosShader = Shader("shaders/postprocess/screenPosShader.vs", "shaders/base/colorShader.fs", "screen pos shader");
    skyShader = Shader("shaders/horizon/skyShader.vs", "shaders/horizon/skyShader.fs", "sky shader");
    
    return flatShader.shadersValid;
}

unsigned int genBuffers(int bufferCount, 
    unsigned int (&VBOs)[], unsigned int (&VAOs)[], unsigned int (&EBOs)[], unsigned int& quadVBO)
{
    glGenBuffers(bufferCount, VBOs);  
    glGenVertexArrays(bufferCount, VAOs);
    glGenBuffers(bufferCount, EBOs);

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(float), &quadVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // Generate all framebuffers
    glGenFramebuffers(FRAMEBUFFER_COUNT, FBOs);
    glGenTextures(FRAMEBUFFER_COUNT, FTexs);

    // Precalculated depth map (trees)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, FBOs[DEPTH_MAP_PRE]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                SHADOW_RESOLUTION, SHADOW_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); 
    // Assume out of range is out of shadow
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, COLOR_WHITE); 

    // Binding texture to buffer
    glBindFramebuffer(GL_FRAMEBUFFER, FBOs[DEPTH_MAP_PRE]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, FTexs[DEPTH_MAP_PRE], 0);

    // Realtime depth map (foliage)
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, FBOs[DEPTH_MAP_RT]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                SHADOW_RESOLUTION / SHADOW_RT_DOWNSCALE, SHADOW_RESOLUTION / SHADOW_RT_DOWNSCALE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); 
    // Assume out of range is out of shadow
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, COLOR_WHITE); 

    // Binding texture to buffer
    glBindFramebuffer(GL_FRAMEBUFFER, FBOs[DEPTH_MAP_RT]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, FTexs[DEPTH_MAP_RT], 0);

    // Other textures (that use color)
    for (int i = 2; i < FRAMEBUFFER_COUNT; i++)
    {
        //glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, FTexs[i]);

            // TODO: WHAT TO DO ABOUT RESIZING?
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
                    SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); 
        glBindTexture(GL_TEXTURE_2D, 0);
    
        glBindFramebuffer(GL_FRAMEBUFFER, FBOs[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FTexs[i], 0);

        if (i <= OCCLUSION_MAP)
        {
            unsigned int rbo;
            glGenRenderbuffers(1, &rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);  
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);  
        }
            
    }
    return 1;
}

unsigned int genTextures()
{
    glGenTextures(1, &skyTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyTexture);

    Image horizonTex = generateMountain(1024);
    for (unsigned int i : {GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                            GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z})
    {
        glTexImage2D(i, 0, GL_RGBA, horizonTex.width, horizonTex.height, 0, 
            GL_RGBA, GL_UNSIGNED_BYTE, horizonTex.data);
    }
    deleteImage(horizonTex);

    Image skyTex = generateSkybox(1024);
    for (unsigned int i : {GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y})
    {
        glTexImage2D(i, 0, GL_RGBA, skyTex.width, skyTex.height, 0, 
            GL_RGBA, GL_UNSIGNED_BYTE, skyTex.data);
    }
    deleteImage(skyTex);
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    Image cloudImage = generatePerlin(2048);

    glGenTextures(1, &cloudTexture);
    glBindTexture(GL_TEXTURE_2D, cloudTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cloudImage.width, cloudImage.height,
        0, GL_RGBA, GL_UNSIGNED_BYTE, cloudImage.data);

    deleteImage(cloudImage);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
    return 1;
}

unsigned int bindBuffer(int bufferIndex, unsigned int (&VBOs)[], unsigned int (&VAOs)[], 
        unsigned int (&EBOs)[], vector<float> vertices, vector<int> indices,
        bool hasInstanceData = false, vector<mat4> instanceData = {})
{
    glBindVertexArray(VAOs[bufferIndex]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[bufferIndex]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[bufferIndex]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);  
    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

    // Position data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Norm data
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Instance data
    if (hasInstanceData)
    {
        unsigned int instanceVBO;
        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(mat4), &instanceData[0], GL_STATIC_DRAW);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)0);
        glEnableVertexAttribArray(2);
        glVertexAttribDivisor(2, 1);  
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)(1 * sizeof(vec4)));
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1);  
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)(2 * sizeof(vec4)));
        glEnableVertexAttribArray(4);
        glVertexAttribDivisor(4, 1);  
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)(3 * sizeof(vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribDivisor(5, 1);  
    }


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return 1;
}

// TODO: Take in extra input
unsigned int genModels(unsigned int (&VBOs)[], unsigned int (&EBOs)[])
{
    // TODO: This crashes randomly?
    for (int i = 0; i < bufferCount; i++)
    {
        PrintLog("Generating model " + to_string(i));
        // TODO: Do I need to be worred about gc?
        vector<float> vertices;
        vector<int> indices;
        int triangleCount;
        ModelData modelData;

        vector<mat4> instanceData;
        if (!createModel(i, vertices, indices, modelData, triangleCount, instanceData,
            designImages, DebugActive(DEBUG_SKIP_DESIGN) ? 0 : designImageCount))
        {
            cerr << "Error when generating model " << i << endl;
            return 0;
        }

        triangleCounts.push_back(triangleCount);
        modelDatas.push_back(modelData);

        if (i & (MODELTYPE_GRASS | MODELTYPE_LEAVES))
            bindBuffer(i, VBOs, VAOs, EBOs, vertices, indices, true, instanceData);
        else
            bindBuffer(i, VBOs, VAOs, EBOs, vertices, indices);
    }


    return 1;
}

void precalc()
{
    glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
    bindFramebuffer(DEPTH_MAP_PRE);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    depthShader.setActive();
    depthShader.setUniform("lightView", lightViewMatrix);
    
    // TODO: Peter panning?
    // No need to render sun for shadowmap
    render(depthShader, 0, MODEL_DEFAULT);

    bindTexture(DEPTH_MAP_PRE);
}

void framebufferSizeCallback(GLFWwindow* window, int newWidth, int newHeight)
{
    // Resize TODO: UNSUPPORTED WITH FRAMEBUFFERS
    // curWidth = newWidth;
    // curHeight = newHeight;
    // glViewport(0, 0, curWidth, curHeight);
    
    // Unused variables
    (void) window;
    (void) newWidth;
    (void) newHeight;
}

// MARK: C Args
unsigned int handleArgs(int argc, char*argv[])
{
    int c;
    while ((c = getopt(argc, argv, "nfchvld")) != -1) {
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
            case 'l':
                SetDebug(DEBUG_DRAW_LIGHTRAYS);
                break;
            case 'd':
                SetDebug(DEBUG_SKIP_DESIGN);
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
    cout << "      -l draw lightrays render" << endl;
    cout << "      -f enable framerate debug" << endl;
    cout << "      -c enable full camera control" << endl;
    cout << "      -d skip design step" << endl;
}