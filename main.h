#include <GLFW/glfw3.h>
#include <string>

using namespace std;

// Initialization
unsigned int handleArgs(int argc, char*argv[]);
void displayHelp();

GLFWwindow* initializeAndCreateWindow(int screenWidth, int screenHeight, const char* windowName);

unsigned int genBuffers(int bufferCount, unsigned int (&VBOs)[], unsigned int (&VAOs)[], unsigned int (&EBOs)[],
    unsigned int &depthFBO, unsigned int &depthMap);

unsigned int bindBuffer(int bufferIndex, unsigned int (&VBOs)[], unsigned int (&VAOs)[], 
        unsigned int (&EBOs)[], vector<float> vertices, vector<int> indices);

// Pipeline
void processInput(GLFWwindow* window);

void render(Shader shader, bool usesNorm, bool usesColor);

// Callback functions
void framebufferSizeCallback(GLFWwindow* window, int newWidth, int newHeight);