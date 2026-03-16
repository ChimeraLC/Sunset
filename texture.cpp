#include <glm/glm.hpp>

#include <iostream>
#include <vector>

#include "consts.h"

#include "texture.h"

using namespace std;

unsigned int timePhase = DUSK;
void setTime(unsigned int newTime) {timePhase = newTime;}
unsigned int getTime() {return timePhase;}

float mountainVar = 0.1f;
float cloudSeed = 0.f;
unsigned int cloudType = CUMULUS;
void setMountainVariance(float variance) { mountainVar = variance; }
void setCloudSeed(float seed) {cloudSeed = seed;}
void setCloudType(unsigned newType) {cloudType = newType; }

float randFloat() {
    return ((float) rand()) / RAND_MAX;    
}

void initPerlin();
float getPerlin(float x, float y, float z);

Image createImage(int width, int height)
{
    unsigned char *data = (unsigned char *)calloc(4 * width * height, sizeof(unsigned char));
    Image image;
    image.width = width;
    image.height = height;
    image.data = data;
    return image;
}

void setColor(Image image, int row, int col, int red, int blue, int green, int alpha)
{
    if (row < image.height && row >= 0 && col < image.width && col >= 0)
    {
    image.data[4 * (row * image.width + col)] =
        (unsigned char)(red);
    image.data[4 * (row * image.width + col) + 1] =
        (unsigned char)(blue);
    image.data[4 * (row * image.width + col) + 2] =
        (unsigned char)(green);
    image.data[4 * (row * image.width + col) + 3] =
        (unsigned char)(alpha);
    }
}

glm::vec3 getColor(Image image, int row, int col)
{
    row = glm::max(glm::min(image.height - 1, row), 0);
    col = glm::max(glm::min(image.width - 1, col), 0);

    return glm::vec3(image.data[4 * (row * image.width + col)],
        image.data[4 * (row * image.width + col) + 1], image.data[4 * (row * image.width + col) + 2]);
}

int getAlpha(Image image, int row, int col)
{
    row = glm::max(glm::min(image.height - 1, row), 0);
    col = glm::max(glm::min(image.width - 1, col), 0);
    return image.data[4 * (row * image.width + col) + 3];
}

void setColor(Image image, int row, int col, glm::ivec3 color, int alpha = 0)
{   setColor(image, row, col, color.r, color.b, color.g, alpha); }

void deleteImage(Image image) { free(image.data); }

glm::vec3 skyColorHigh = glm::vec3(50, 150, 130);
glm::vec3 skyColorHighNight = glm::vec3(5, 20, 10);
glm::vec3 starColor = glm::vec3(200, 200, 200);
glm::vec3 skyColorLow = glm::vec3(190, 100, 100);

Image generateSkybox(int width)
{
    Image image = createImage(width, width);

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (timePhase == NIGHT)
                setColor(image, i, j, skyColorHighNight, 0);
            else if (timePhase == DUSK)
                setColor(image, i, j, (skyColorHigh + skyColorLow) / 2.0f, 0);
        }
    }

    // Generating stars
    if (timePhase == NIGHT)
    {
        for (int i = 0; i < 25; i++)
        {
            int row = randFloat() * width;
            int col = randFloat() * width;
            int radius = 1 + randFloat() * 3;
            for (int i = -radius; i <= radius; i++)
            {
                float remaining = sqrt(radius * radius - i * i);
                for (int j = -remaining; j <= remaining; j++)
                {
                    setColor(image, col + i, row + j, starColor, 0);
                }
            }
        }
    }
    return image;
}

Image generatePerlin(int width)
{
    Image image = createImage(width, width);

    initPerlin();

    for (int i = 0; i < width / 2; i++)
    {
        for (int j = 0; j < width / 2; j++)
        {
            float noiseVal;
            switch (cloudType)
            {
                case CUMULUS:
                    noiseVal = getPerlin((float) i / (width / 8), (float) j / (width / 8), cloudSeed);
                    break;
                case STRATUS:
                    noiseVal = getPerlin((float) i / (width / 4), (float) j / (width / 24), cloudSeed);
                    break;
                default:
                    noiseVal = 0;
            }
            int scaledNoise = noiseVal * 192 + 128;
            scaledNoise = clamp(scaledNoise, 0, 255);
            setColor(image, i, j, scaledNoise, scaledNoise, scaledNoise, scaledNoise);
            setColor(image, width - 1 - i, j, scaledNoise, scaledNoise, scaledNoise, scaledNoise);
            setColor(image, width - 1 - i, width - 1 - j, scaledNoise, scaledNoise, scaledNoise, scaledNoise);
            setColor(image, i, width - 1 - j, scaledNoise, scaledNoise, scaledNoise, scaledNoise);
        }
    }

    return image;
}

Image generateMountain(int width)
{
    Image image = createImage(width, width);

    // Horizontal position, vertical position pairs 
    int peakCount = 4;
    vector<float> peaks = {
        -0.8, 0.53,
        0.1, 0.53f + mountainVar / 10,
        0.3, 0.53f - mountainVar / 10,
        0.6, 0.53f + mountainVar / 10,
        0.8, 0.53f,
        1.1, 0.53f + mountainVar / 10,
    };

    for (int col = 0; col < width; col++)
    {
        //Inefficient version
        float horizontalPos = ((float) col) / width;
        int mountainHeight;
        float left = 0, right, leftHeight, rightHeight;
        for (int i = 1; i < peakCount + 2; i++)
        {
            if (horizontalPos < peaks[i * 2])
            {
                right = peaks[i * 2];
                rightHeight = peaks[i * 2 + 1];
                left = peaks[(i - 1) * 2];
                leftHeight = peaks[(i - 1) * 2 + 1];
                break;
            }
        }

        // How 'flat' slops of mountains are
        float slopeDamp = mountainVar;

        float maxHeight = glm::max(leftHeight, rightHeight);
        float minHeight = glm::min(leftHeight, rightHeight);

        float fullHeight = maxHeight - minHeight;
        float availableHeight = (right - left) * slopeDamp;

        float lowHeight = minHeight - (availableHeight - fullHeight) / 2;
        float low = (leftHeight - lowHeight) / slopeDamp + left;

        // TODO: Where did the math end up 1-x?
        mountainHeight = width * (abs(horizontalPos - low) * slopeDamp + lowHeight);

        //std::cout << horizontalPos << " " << left << " " << right << " " << (abs(horizontalPos - low) + lowHeight) << std::endl;

        for (int row = 0; row < width; row++)
        {
            if (row <= mountainHeight)        
            {
                int closeness = glm::max(10 * width / (mountainHeight) - (mountainHeight - row), -10);
                setColor(image, width - row - 1, col, 24 + closeness, 10 + closeness, 10 + closeness, 255);
            }
            else
            {
                float heightFrac = timePhase == NIGHT ? glm::min((float) row / width + 0.3f, 1.0f) : (float) row / width / 2;
                setColor(image, width - row - 1, col, 
                    heightFrac * (timePhase == NIGHT ? skyColorHighNight : skyColorHigh) + (1 - heightFrac) * skyColorLow, 0);    
            }
        }
    }

    // Generating stars
    if (timePhase == NIGHT)
    {
        for (int i = 0; i < 15; i++)
        {
            int row = (randFloat() * 0.35f) * width;
            int col = randFloat() * width;
            int radius = 1 + randFloat() * 2;
            for (int i = -radius; i <= radius; i++)
            {
                float remaining = sqrt(radius * radius - i * i);
                for (int j = -remaining; j <= remaining; j++)
                {
                    int curRow = row + j;
                    float heightFrac = glm::min((float) curRow / width + 0.3f, 1.0f);
                    if (getAlpha(image, curRow, col+i) < 0.5f)
                        setColor(image, curRow, col+i, starColor * (1 - heightFrac) + heightFrac *
                            getColor(image, curRow, col+i), 0);
                }
            }
        }
    }
    return image;
}

Image generateEmpty(int width, int height)
{
    Image image = createImage(width, height);
    
    for (int col = 0; col < width; col++)
    {
        for (int row = 0; row < height; row++)
        {
            setColor(image, row, col, glm::ivec3(255, 255, 255));
        }
    }
    return image;
}

// MARK: Perlin
int permutation[] = { 151,160,137,91,90,15, // When the goat (Ken Perlin) speaks, I listen
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

int p[512];
void initPerlin() {
    for (int i = 0; i < 256; i++)
    {
        p[i] = permutation[i];
        p[i + 256] = permutation[i];
    }
}

float fade(float t) {return t * t * t * (t * (t * 6 - 15) + 10); }
float lerp(float t, float a, float b) { return a + t * (b-a); }
float grad(int hs, float x, float y, float z) {
    int h = hs & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : ( h==12 || h==14 ? x : z);
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

float getPerlin(float x, float y, float z) {
    int X = ((int) floor(x)) & 255;
    int Y = ((int) floor(y)) & 255;
    int Z = ((int) floor(z)) & 255;
    x -= floor(x);
    y -= floor(y);
    z -= floor(z);
    float u = fade(x);
    float v = fade(y);
    float w = fade(z);
    int A = p[X] + Y; int AA = p[A] + Z; int AB = p[A+1] + Z;
    int B = p[X+1] + Y; int BA = p[B] + Z; int BB = p[B+1] + Z;

    return lerp(w, lerp(v, lerp(u, grad(p[AA  ], x  , y  , z   ),
                                     grad(p[BA  ], x-1, y  , z   )),
                             lerp(u, grad(p[AB  ], x  , y-1, z   ),
                                     grad(p[BB  ], x-1, y-1, z   ))),
                     lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1 ),
                                     grad(p[BA+1], x-1, y  , z-1 )),
                             lerp(u, grad(p[AB+1], x  , y-1, z-1 ),
                                     grad(p[BB+1], x-1, y-1, z-1 ))));
}