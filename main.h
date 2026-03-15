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
unsigned int genTextures();

unsigned int bindBuffer(int bufferIndex, unsigned int (&VBOs)[], unsigned int (&VAOs)[], 
        unsigned int (&EBOs)[], vector<float> vertices, vector<int> indices,
        bool hasInstanceData, vector<float> instanceData);

enum TextureBuffer {
    DEPTH_MAP_PRE,
    DEPTH_MAP_RT,
    POSTPROCESS,    
    OCCLUSION_MAP,  // Everything before Occlusion Map gets a depth buffer
    LIGHTRAYS_MAP,
    TEMPORARY_A,
    BLOOM,
    SKYBOX,         // Prerendered textures
    FRAMEBUFFER_COUNT,
};
void bindFramebuffer(TextureBuffer buffer);
void bindTexture(TextureBuffer buffer);
unsigned int genModels( unsigned int (&VBOs)[], unsigned int (&EBOs)[]);
void precalc();

// Pipeline
void processInput(GLFWwindow* window);

enum RenderFlags {
    RENDER_NORM = 0x01,
    RENDER_COLOR = 0x02,
    RENDER_LIGHTOCCLUSION = 0x04,
    TIME_DEPENDENT = 0x08,
    INSTANCED = 0x10,
    CULL_DISABLED = 0x20,
};

void render(Shader shader, unsigned int renderflags, unsigned int drawflags);
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