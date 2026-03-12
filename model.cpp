#include <glm/glm.hpp>

#include <vector>
#include <tuple>
#include <random>
#include <iostream>

#include "consts.h"

#include "model.h"

using namespace glm;
using namespace std;


unsigned int createModel(int index, vector<float>& vertices, vector<int>& indices, 
    ModelData& modelData, int& triangleCount) {
    switch (index)
    {
        case 0:
            createModelSun(vertices, indices, modelData, triangleCount);
            break;
        case 1:
            createModelGround(vertices, indices, modelData, triangleCount);
            break;
        case 2:
            createModelTrunk(vertices, indices, modelData, triangleCount, vec3(0.2, 0, 0));
            break;
        case 3:
            createModelTrunk(vertices, indices, modelData, triangleCount, vec3(-0.2, 0, 0));
            break;
        default:
            return 0;
    }
    return 1;
}

float randFloat() {
    return ((float) rand()) / RAND_MAX;    
}

void fillVertexNormals(vector<float> const& preVertices, 
        vector<int> const& preIndices,
        vector<float>& vertices, vector<int>& indices,
        int& triangleCount) {

    triangleCount = preIndices.size() / 3;

    for (int i = 0; i < triangleCount; i++)
    {
        int triIndices[] = {preIndices[i * 3], 
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

void pushTriangle(vector<float>& vertices, float x, float y, float z)
{
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z);
}
void pushTriangle(vector<float>& vertices, vec3 newPoint)
{
    vertices.push_back(newPoint.x);
    vertices.push_back(newPoint.y);
    vertices.push_back(newPoint.z);
}

void pushIndices(vector<int>& indices, int index1, int index2, int index3)
{
    indices.push_back(index1);
    indices.push_back(index2);
    indices.push_back(index3);
}

void displayValues(vector<float>& preVertices, vector<int>& preIndices)
{
    std::cout << "\nVertices:" << endl;
    for (int i = 0; i < (int) preVertices.size(); i++)
    {
        if (i > 0 && i % 3 == 0)
            std::cout << std::endl;
        std::cout << preVertices[i] << ", ";
    }
    std::cout << "\nIndices:" << endl;
    for (int i = 0; i < (int) preIndices.size(); i++)
    {
        if (i > 0 && i % 3 == 0)
            std::cout << std::endl;
        std::cout << preIndices[i] << ", ";
    }
}

void createModelSun(vector<float>& vertices, vector<int>& indices, 
    ModelData& modelData, int& triangleCount) {

    modelData.modelType |= MODEL_LIGHTSOURCE;

    int sunSides = 12;
    vector<float> preVertices;
    vector<int> preIndices;
    
    float angle = PI * 2 / sunSides;
    for (int i = 0; i < sunSides; i++)
    {
        pushTriangle(preVertices, vec3(cos(angle * (i + 0.5)), sin(angle * (i + 0.5)), 0));
        pushIndices(preIndices, i, i + 1, sunSides - 1);
    }


    fillVertexNormals(preVertices, preIndices, vertices, indices, triangleCount);
    modelData.color = vec3(0.95f, 0.9f, 0.6f);
}

void createModelGround(vector<float>& vertices, vector<int>& indices, 
    ModelData& modelData, int& triangleCount) {
        
    modelData.modelType |= MODEL_DEFAULT;

    vector<float> preVertices = {
            1.0, 0.0f, 1.0f,
            1.0, 0.0f, -1.0f, 
            -1.0f, 0.0f, -1.0f,
            -1.0, 0.0f, 1.0f, 
        };

    vector<int> preIndices = {
            0, 1, 3,
            1, 2, 3
        };

    fillVertexNormals(preVertices, preIndices, vertices, indices, triangleCount);
    modelData.color = vec3(0.0f, 0.3f, 0.0f);
}

// ████████╗██████╗ ███████╗███████╗
// ╚══██╔══╝██╔══██╗██╔════╝██╔════╝
//    ██║   ██████╔╝█████╗  █████╗  
//    ██║   ██╔══██╗██╔══╝  ██╔══╝  
//    ██║   ██║  ██║███████╗███████╗
//    ╚═╝   ╚═╝  ╚═╝╚══════╝╚══════╝

// Creates a tree ring of points; returns start point of the ring
// Currently is not orriented any way
int createTreeRing(vec3 ringCenter, float radius, int sides, vector<float>& vertices)
{
    int startIndex = vertices.size() / 3;

    float angle = -2 * PI / sides; // Wind clockwise
    for (int i = 0; i < sides; i++)
    {
        vec3 offset = radius * vec3(cos(angle * i), 0, sin(angle * i));
        pushTriangle(vertices, ringCenter + offset);
    }

    return startIndex;
}

// Adds the necessary triangles to connect two tree rings
// Currently assumes sides and orrentation is the same
void connectTreeRings(int indexA, int indexB, int sides,
        vector<int>& indices)
{
    for (int i = 0; i < sides; i++)
    {
        int left = i;
        int right = (i + 1) % sides;
        pushIndices(indices, indexA + left, indexA + right, indexB + left);
        pushIndices(indices, indexA + right, indexB + right, indexB + left);
    }
}

//Ends off a tree branch; should work the same as connectTreeRings with sidesB = 1
// Index A is a ring, index B is a single point
void capBranch(vec3 branchEnd, int indexA, int sides,
        vector<float>&vertices, vector<int>& indices)
{
    int startIndex = vertices.size() / 3;

    pushTriangle(vertices, branchEnd);

    for (int i = 0; i < sides; i++)
    {
        int left = i;
        int right = (i + 1) % sides;
        pushIndices(indices, indexA + left, indexA + right, startIndex);
    }
}

// // Splits a ring into two branch points
// void splitTreeRing(int index, int& outIndexA, int& outIndexB, int sides,
//     vector<float>&vertices, vector<int>& indices)
// {
// }

void createModelTrunk(vector<float>& vertices, vector<int>& indices, 
    ModelData& modelData, int& triangleCount, vec3 offset = vec3(0)) {

    modelData.modelType |= MODEL_DEFAULT;
    modelData.color = vec3(0.6f, 0.3f, 0.0f);
    modelData.translation = offset;
    
    int sides = 10;

    vector<float> preVertices;
    vector<int> preIndices;

    int indexA = createTreeRing(vec3(0), 0.1f, sides, preVertices);
    int indexB = createTreeRing(vec3(0, 0.3, 0), 0.08f, sides, preVertices);
    int indexC = createTreeRing(vec3(0, 0.5, 0), 0.05f, sides, preVertices);
    connectTreeRings(indexA, indexB, sides, preIndices);
    connectTreeRings(indexB, indexC, sides, preIndices);
    capBranch(vec3(0.1f, 0.6f, 0.0f), indexC, sides, preVertices, preIndices);

    fillVertexNormals(preVertices, preIndices, vertices, indices, triangleCount);
}