#include <GLFW/glfw3.h>
#include <string>

using namespace std;

// Initialization
GLFWwindow* initializeAndCreateWindow(int screenWidth, int screenHeight, const char* windowName);
unsigned int linkBuffers(unsigned int& VBO, unsigned int& VAO, unsigned int& EBO);

// Pipeline
void processInput(GLFWwindow* window);

// Callback functions
void framebufferSizeCallback(GLFWwindow* window, int newWidth, int newHeight);