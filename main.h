#include <GLFW/glfw3.h>
#include <string>

using namespace std;

// Initialization
unsigned int handleArgs(int argc, char*argv[]);
void displayHelp();

GLFWwindow* initializeAndCreateWindow(int screenWidth, int screenHeight, const char* windowName);

bool compileShaders();

unsigned int genBuffers(int bufferCount, 
    unsigned int (&VBOs)[], unsigned int (&VAOs)[], unsigned int (&EBOs)[], unsigned int& quadVBO);

unsigned int bindBuffer(int bufferIndex, unsigned int (&VBOs)[], unsigned int (&VAOs)[], 
        unsigned int (&EBOs)[], vector<float> vertices, vector<int> indices);

enum TextureBuffer {
    DEPTH_MAP,
    OCCLUSION_MAP,
    LIGHTRAYS_MAP,
    POSTPROCESS,
    TEMPORARY_A,
    BLOOM,
    FRAMEBUFFER_COUNT,
};
void bindFramebuffer(TextureBuffer buffer);
void bindTexture(TextureBuffer buffer);

// Pipeline
void processInput(GLFWwindow* window);

enum RenderFlags {
    RENDER_NORM = 0x01,
    RENDER_COLOR = 0x02,
    RENDER_LIGHTOCCLUSION = 0x04,
};

void render(Shader shader, unsigned int renderflags, unsigned int drawflags = ~0);
void renderScreenQuad(Shader shader);

// Callback functions
void framebufferSizeCallback(GLFWwindow* window, int newWidth, int newHeight);


// Generic quad
std::vector<float> quadVertices = {
	-1,  1,   0, 1,
	-1, -1,   0, 0,
	 1, -1,   1, 0,

	-1,  1,   0, 1,
	 1, -1,   1, 0,
	 1,  1,   1, 1
};