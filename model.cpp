#include <vector>

#include "model.h"

using namespace std;

unsigned int createModel(int index, vector<float>& vertices, vector<int>& indices, int& triangleCount) {
    switch (index)
    {
        case 0:
            createModelGround(vertices, indices, triangleCount);
            break;
        case 1:
            createModelTrunk(vertices, indices, triangleCount);
            break;
        default:
            return 0;
    }
    return 1;
}

void createModelGround(vector<float>& vertices, vector<int>& indices, int& triangleCount) {
    vertices = {
            1.0, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            1.0, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            -1.0, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f
        };

    indices = {
            0, 1, 3,
            1, 2, 3
        };

    triangleCount = 2;
}
void createModelTrunk(vector<float>& vertices, vector<int>& indices, int& triangleCount) {
    // TODO: Automatically calculate normals
    vertices = {
            0.5f, 0.0f, 0.5f, 0.8f, 0.6f, 0.0f,
            0.5f, 0.0f, -0.5f, 0.8f, 0.6f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.8f, 0.6f, 0.0f,
            0.5f, 0.0f, -0.5f, 0.0f, 0.6f, -0.8f,
            -0.5f, 0.0f, -0.5f, 0.0f, 0.6f, -0.8f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.6f, -0.8f,
            -0.5f, 0.0f, -0.5f, -0.8f, 0.6f, 0.0f,
            -0.5f, 0.0f, 0.5f, -0.8f, 0.6f, 0.0f,
            0.0f, 1.0f, 0.0f, -0.8f, 0.6f, 0.0f,
            -0.5f, 0.0f, 0.5f, 0.0f, 0.6f, 0.8f,
            0.5f, 0.0f, 0.5f, 0.0f, 0.6f, 0.8f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.6f, 0.8f,
        };

    indices = {
            0, 1, 2,
            3, 4, 5,
            6, 7, 8,
            9, 10, 11
        };

    triangleCount = 4;
}