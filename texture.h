#include <cstdlib>

// Handles generating textures at runtime
/* Image struct */
typedef struct
{
    int width;
    int height;
    unsigned char *data;
} Image;

void deleteImage(Image image);

Image generateSkybox(int width);
Image generateMountain(int width);