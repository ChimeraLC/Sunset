#include <glm/glm.hpp>

#include <iostream>
#include <vector>

#include "texture.h"

using namespace std;

Image createImage(int width, int height)
{
    unsigned char *data = (unsigned char *)calloc(4 * width * height, sizeof(unsigned char));
    Image image;
    image.width = width;
    image.height = width;
    image.data = data;
    return image;
}

void setColor(Image image, int row, int col, int red, int blue, int green, int alpha = 0)
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
void setColor(Image image, int row, int col, glm::ivec3 color, int alpha = 0)
{   setColor(image, row, col, color.r, color.b, color.g, alpha); }

void deleteImage(Image image) { free(image.data); }

glm::vec3 skyColorHigh = glm::vec3(50, 150, 130);
glm::vec3 skyColorHighNight = glm::vec3(5, 20, 10);
glm::vec3 skyColorLow = glm::vec3(190, 100, 100);
const bool isNight = false;

Image generateSkybox(int width)
{
    Image image = createImage(width, width);
    
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (isNight)
                setColor(image, i, j, skyColorHighNight, 0);
            else
                setColor(image, i, j, (skyColorHigh + skyColorLow) / 2.0f, 0);
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
        0.1, 0.55,
        0.3, 0.51,
        0.6, 0.55,
        0.8, 0.53,
    };

    // Putting 'bumpers' on front and back
    peaks.push_back(peaks[0] + 1);
    peaks.push_back(peaks[1]);
    peaks.insert(peaks.begin(), peaks[-3]);
    peaks.insert(peaks.begin(), peaks[-4] - 1);

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
        float slopeDamp = 0.2f;

        float maxHeight = max(leftHeight, rightHeight);
        float minHeight = min(leftHeight, rightHeight);

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
                setColor(image, width - row - 1, col, 12, 4, 4, 255);
            else
            {
                float heightFrac = isNight ? min((float) row / width + 0.3f, 1.0f) : (float) row / width / 2;
                setColor(image, width - row - 1, col, 
                    heightFrac * (isNight ? skyColorHighNight : skyColorHigh) + (1 - heightFrac) * skyColorLow, 0);    
            }
        }
    }

    return image;
}

