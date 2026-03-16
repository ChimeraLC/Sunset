#include <cstdlib>

#include <glm/glm.hpp>

#ifndef TEXTURE_H
#define TEXTURE_H
// Handles generating textures at runtime
/* Image struct */
typedef struct
{
    int width;
    int height;
    unsigned char *data;
} Image;

float randFloat();
void setCloudSeed(float seed);

enum cloudTypes {CLEARDAY, STRATUS, CUMULUS};
void setCloudType(unsigned int type);
void setMountainVariance(float variance);
void setTime(unsigned int newTime);
unsigned int getTime();
void deleteImage(Image image);
void setColor(Image image, int row, int col, int red, int blue, int green, int alpha = 0);
glm::vec3 getColor(Image image, int row, int col);

Image generateSkybox(int width);
Image generateMountain(int width);
Image generateEmpty(int width, int height);
Image generatePerlin(int width);

#endif // TEXTURE_H