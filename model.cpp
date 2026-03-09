#include <glm/glm.hpp>

#include <vector>
#include <tuple>

#include "model.h"

using namespace glm;
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

    triangleCount = 4;
    
    vector<float> preVertices = {
            0.5f, 0.0f, 0.5f, 
            0.5f, 0.0f, -0.5f, 
            -0.5f, 0.0f, -0.5f, 
            -0.5f, 0.0f, 0.5f, 
            0.0f, 1.0f, 0.0f, 
    };

    vector<int> preIndices = {
        0, 1, 4,
        1, 2, 4,
        2, 3, 4,
        3, 0, 4
    };

    fillVertexNormals(preVertices, preIndices, vertices, indices, triangleCount);
}

void fillVertexNormals(vector<float> const& preVertices, 
        vector<int> const& preIndices,
        vector<float>& vertices, vector<int>& indices,
        int triangleCount) {

    for (int i = 0; i < triangleCount; i++)
    {
        int triIndices[] = {preIndices[i * 3 ], 
            preIndices[i * 3 + 1], preIndices[i * 3 + 2]};

        // Calculate normals
        vec3 normal = getNormal(&preVertices[triIndices[0] * 3],
            &preVertices[triIndices[1] * 3], &preVertices[triIndices[2] * 3]);

        // Push onto final vertices list
        for (int j = 0; j < 3; j++)
        {
            vertices.push_back(preVertices[triIndices[j] * 3]);
            vertices.push_back(preVertices[triIndices[j] * 3+1]);
            vertices.push_back(preVertices[triIndices[j] * 3+2]);
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
        }

        // Assign unique indices for every single new triangle
        indices.push_back(i*3);
        indices.push_back(i*3+1);
        indices.push_back(i*3+2);
    }
}

// TODO: Does using pointers here really make it that much cleaner
vec3 getNormal(const float* point1, const float* point2, const float* point3)
{
    vec3 side1 = vec3(point2[0] - point1[0], point2[1] - point1[1], point2[2] - point1[2]);
    vec3 side2 = vec3(point3[0] - point2[0], point3[1] - point2[1], point3[2] - point2[2]);
    return normalize(cross(side1, side2));
}