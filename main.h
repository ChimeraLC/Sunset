#include <GLFW/glfw3.h>
#include <string>

using namespace std;

// Initialization
unsigned int handleArgs(int argc, char*argv[]);
void displayHelp();

GLFWwindow* initializeAndCreateWindow(int screenWidth, int screenHeight, const char* windowName);

unsigned int genBuffers(int bufferCount, unsigned int (&VBOs)[], unsigned int (&VAOs)[], unsigned int (&EBOs)[],
    unsigned int &depthFBO, unsigned int &depthMap, unsigned int& occlusionFBO, unsigned int& occlusionMap);

unsigned int bindBuffer(int bufferIndex, unsigned int (&VBOs)[], unsigned int (&VAOs)[], 
        unsigned int (&EBOs)[], vector<float> vertices, vector<int> indices);

// Pipeline
void processInput(GLFWwindow* window);

enum RenderFlags {
    RENDER_NORM = 0x01,
    RENDER_COLOR = 0x02,
    RENDER_LIGHTOCCLUSION = 0x04,
};

void render(Shader shader, unsigned int renderflags, unsigned int drawflags = ~0);

// Callback functions
void framebufferSizeCallback(GLFWwindow* window, int newWidth, int newHeight);