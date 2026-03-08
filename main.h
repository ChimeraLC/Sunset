#include <GLFW/glfw3.h>
#include <string>

using namespace std;

// Initialization
GLFWwindow* initializeAndCreateWindow(int screenWidth, int screenHeight, const char* windowName);

unsigned int genBuffers(int bufferCount, unsigned int (&VBOs)[], unsigned int (&VAOs)[], unsigned int (&EBOs)[]);

unsigned int bindBuffer(int bufferIndex, unsigned int (&VBOs)[], unsigned int (&VAOs)[], 
        unsigned int (&EBOs)[], vector<float> vertices, vector<int> indices);

// Pipeline
void processInput(GLFWwindow* window);

// Callback functions
void framebufferSizeCallback(GLFWwindow* window, int newWidth, int newHeight);