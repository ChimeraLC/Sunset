#include "texture.h"

#include <iostream>
void deleteImage(Image image) { free(image.data); }

Image generateSkybox(int width)
{
    unsigned char *data = (unsigned char *)calloc(3 * width * width, sizeof(unsigned char));

    // Currently just all blue
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < width; j++)
        {
            // Red
            data[3 * (i * width + j)] =
                (unsigned char)(40);
            // Green
            data[3 * (i * width + j) + 1] =
                (unsigned char)(120);
            // Blue
            data[3 * (i * width + j) + 2] =
                (unsigned char)(150);
        }
    }
    
    Image image;
    image.width = width;
    image.height = width;
    image.data = data;

    return image;

}

