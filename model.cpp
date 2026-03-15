#include <glm/glm.hpp>

#include <vector>
#include <tuple>
#include <random>
#include <iostream>

#include "consts.h"

#include "model.h"

using namespace glm;
using namespace std;

const vec3 VECTOR_ALIGN = vec3(1, 0, 0);

unsigned int createModel(int index, vector<float>& vertices, vector<int>& indices, 
    ModelData& modelData, int& triangleCount) {
    switch (index)
    {
        case MODELTYPE_SUN:
            createModelSun(vertices, indices, modelData, triangleCount);
            break;
        case MODELTYPE_SKYBOX:
            createModelSkybox(vertices, indices, modelData, triangleCount);
            break;
        case MODELTYPE_GROUND:
            createModelGround(vertices, indices, modelData, triangleCount);
            break;
        case MODELTYPE_TRUNK:
            createModelTrunk(vertices, indices, modelData, triangleCount);
            break;
        case MODELTYPE_GRASS:
            createModelGrass(vertices, indices, modelData, triangleCount);
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

vec3 getVertex(vector<float>& vertices, int index)
{
    return vec3(vertices[index * 3], vertices[index * 3 + 1], vertices[index * 3 + 2]);
}

// TODO: Does using pointers here really make it that much cleaner
vec3 getNormal(const float* point1, const float* point2, const float* point3)
{
    vec3 side1 = vec3(point2[0] - point1[0], point2[1] - point1[1], point2[2] - point1[2]);
    vec3 side2 = vec3(point3[0] - point2[0], point3[1] - point2[1], point3[2] - point2[2]);
    return normalize(cross(side1, side2));
}

void pushVertex(vector<float>& vertices, float x, float y, float z)
{
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z);
}
void pushVertex(vector<float>& vertices, vec3 newPoint)
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
        pushVertex(preVertices, vec3(cos(angle * (i + 0.5)), sin(angle * (i + 0.5)), 0));
        pushIndices(preIndices, i, i + 1, sunSides - 1);
    }


    fillVertexNormals(preVertices, preIndices, vertices, indices, triangleCount);
    modelData.color = vec3(0.95f, 0.9f, 0.6f);
}

void createModelGround(vector<float>& vertices, vector<int>& indices, 
    ModelData& modelData, int& triangleCount) {
        
    modelData.modelType |= MODEL_DEFAULT;

    float groundBounds = 10.0f;

    vector<float> preVertices = {
            0.0, 0.0, 0.0,
            groundBounds, 0.0f, groundBounds,
            groundBounds, 0.0f, -groundBounds, 
            -groundBounds, 0.0f, -groundBounds,
            -groundBounds, 0.0f, groundBounds, 
        };

    vector<int> preIndices = {
            0, 1, 2,
            0, 2, 3,
            0, 3, 4,
            0, 4, 1,
        };

    fillVertexNormals(preVertices, preIndices, vertices, indices, triangleCount);
    modelData.color = vec3(0.0f, 0.3f, 0.0f);
}

void createModelSkybox(vector<float>& vertices, vector<int>& indices, 
    ModelData& modelData, int& triangleCount) {
        
    modelData.modelType |= MODEL_SKYBOX;

    vector<float> preVertices = {
        -1, -1, -1,
        -1, -1, 1,
        -1, 1, -1,
        -1, 1, 1,
        1, -1, -1,
        1, -1, 1,
        1, 1, -1,
        1, 1, 1
    };

    vector<int> preIndices = {
            2, 0, 4,
            2, 4, 6,

            1, 0, 2,
            1, 2, 3,

            4, 5, 7,
            4, 7, 6,

            1, 3, 7,
            1, 7, 5,

            2, 6, 7,
            2, 7, 3,

            4, 0, 1,
            4, 1, 5,
        };

    fillVertexNormals(preVertices, preIndices, vertices, indices, triangleCount);
}

void createModelGrass(vector<float>& vertices, vector<int>& indices, 
    ModelData& modelData, int& triangleCount) {
        
    modelData.modelType |= MODEL_FOLIAGE;
    modelData.translation = vec3(0, 0, 0.3);

    vector<float> preVertices = {
        0.03, 0, 0,
        -0.03, 0, 0,
        0, 0.22, 0,
    };

    vector<int> preIndices = {
            0, 1, 2
    };

    fillVertexNormals(preVertices, preIndices, vertices, indices, triangleCount);

    modelData.color = vec3(0.0f, 0.5f, 0.0f);
    modelData.instanceCount = 100;
}
// MARK: TREE

// Creates a tree ring of points; returns start point of the ring
// Currently is not orriented any way
int createTreeRing(vec3 ringCenter, vec3 inDirection, float radius, int sides, vector<float>& vertices)
{
    // First index is always aligned along vec3(1, 0, 0)
    vec3 right = normalize(cross(inDirection, VECTOR_ALIGN));
    vec3 forward = -normalize(cross(inDirection, right));

    int startIndex = vertices.size() / 3;

    float angle = 2 * PI / sides; // Wind clockwise
    for (int i = 0; i < sides; i++)
    {
        vec3 offset = radius * (cos(angle * i) * forward + sin(angle * i) * right);
        pushVertex(vertices, ringCenter + offset);
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

    pushVertex(vertices, branchEnd);

    for (int i = 0; i < sides; i++)
    {
        int left = i;
        int right = (i + 1) % sides;
        pushIndices(indices, indexA + left, indexA + right, startIndex);
    }
}

//Splits a ring into two branch points (split direction is perpendicular to line split on)
void splitTreeRing(int index, vec3 splitDirection, int& outIndexA, int& outIndexB, int sides,
    vector<float>&vertices, vector<int>& indices)
{
    // These values are always a given
    int startIndex = vertices.size() / 3;
    
    outIndexA = startIndex;
    outIndexB = startIndex + sides;
    
    // Find split index point
    vec3 center = vec3(0);
    for (int i = 0; i < sides; i++) {center += getVertex(vertices, index + i);}
    center /= sides;

    float closestDot = -FLT_MAX; int closest = 0;
    float alignedDot = -FLT_MAX; int mostAligned = 0; // Also track most aligned to (1, 0, 0);
    splitDirection = normalize(splitDirection);
    for (int i = 0; i < sides; i++) {
        vec3 vecDirection = normalize(getVertex(vertices, index + i) - center);
        float alignment = dot(vecDirection, splitDirection);
        if (alignment > closestDot)
        {
            closestDot = alignment;
            closest = i;
        }
        alignment = dot(vecDirection, VECTOR_ALIGN);
        if (alignment > alignedDot)
        {
            alignedDot = alignment;
            mostAligned = 0;
        }
    }

    vec3 forward = normalize(getVertex(vertices, index + closest) - center);
    int rightPart, lowNew, highNew;
    vec3 newPoints[sides / 2]; // generous bounds, since %4==0 uses one less

    if (sides%4 == 0)
    {
        // Winds couterclockwise
        rightPart = index + (closest + sides / 4) % sides;
        vec3 right = getVertex(vertices, rightPart) - center;
        float radius = length(right);
        right = right / radius;

        // Create extra point
        vec3 up = cross(forward, right);

        // Add two ring sets, Aligned towards (1, 0, 0)
        lowNew = sides / 4 + 1; highNew = 3 * sides / 4 - 1;

        float angle = 2 * PI / sides;
        for (int i = 0; i <= highNew - lowNew; i++)
        {
            newPoints[i] = center + radius * (cos(angle * (i + 1)) * right + sin(angle * (i + 1) * up));
        }
    }
    else // sides % 4 == 2
    {
        // Winds couterclockwise
        rightPart = index + (closest + (sides - 2) / 4) % sides;
        int leftPart = index + (closest + (3 * sides - 2) / 4) % sides;
        vec3 right = (getVertex(vertices, rightPart) + getVertex(vertices, rightPart + 1)) / 2.0f - center;
        float radius = length(right);
        right = right / radius;

        // Create extra point
        vec3 up = cross(forward, right);

        // Add two ring sets, Aligned towards (1, 0, 0)
        lowNew = (sides + 2) / 4; highNew = (3 * sides - 2) / 4;

        float angle = 2 * PI / (sides + 2);
        for (int i = 0; i <= highNew - lowNew; i++)
        {
            newPoints[i] = center + radius * (cos(angle * (i + 1)) * right + sin(angle * (i + 1) * up));
        }
        
        // Push fillin triangles
        pushIndices(indices, rightPart, rightPart + 1, startIndex + (lowNew - mostAligned + sides) % sides); 
        pushIndices(indices, leftPart, leftPart + 1, startIndex + (highNew - mostAligned + sides) % sides); 
    }

    
    // Aligning towards (1, 0, 0); making an estimate based on pre-split ring
    int offset = mostAligned;
    // Forward side partition
    for (int i = 0; i < sides; i++)
    {   
        int trueIndex = (offset + i) % sides;
        if (trueIndex >= lowNew && trueIndex <= highNew)
            pushVertex(vertices, newPoints[trueIndex - lowNew]);
        else
            pushVertex(vertices, getVertex(vertices, index + (closest + trueIndex) % sides));
    }

    offset = (mostAligned + (sides / 2)) % sides;
    // Reverse side partition
    for (int i = 0; i < sides; i++)
    {
        int trueIndex = (offset + i) % sides;
        if (trueIndex >= lowNew && trueIndex <= highNew)
        {
            // These newPoints go backwards
            pushVertex(vertices, newPoints[highNew - trueIndex]);
        }
        else
            pushVertex(vertices, getVertex(vertices, index + (closest + trueIndex + sides / 2) % sides));
    }

}

void createModelTrunk(vector<float>& vertices, vector<int>& indices, 
    ModelData& modelData, int& triangleCount) {

    modelData.modelType |= MODEL_DEFAULT;
    modelData.color = vec3(0.6f, 0.3f, 0.0f);
    modelData.translation = vec3(0.0f);;
    
    int sides = 12;

    if (sides % 2 == 1)
        cerr << "sides cannot be odd" << endl;

    vector<float> preVertices;
    vector<int> preIndices;

    int indexA = createTreeRing(vec3(0), vec3(0, 1, 0), 0.1f, sides, preVertices);
    int indexB = createTreeRing(vec3(0.2, 0.5, 0), vec3(0.25, 1, 0), 0.08f, sides, preVertices);
    // int indexC = createTreeRing(vec3(0, 0.5, 0), 0.05f, sides, preVertices);
    
    int indexC, indexD;
    splitTreeRing(indexA, vec3(0.5, 0, 0.5), indexC, indexD, sides, preVertices, preIndices);
    //connectTreeRings(indexA, indexB, sides, preIndices);
    connectTreeRings(indexC, indexB, sides, preIndices);

    capBranch(vec3(0.6f, 0.9f, 0.0f), indexB, sides, preVertices, preIndices);
    capBranch(vec3(-0.1f, 0.9f, 0.0f), indexD, sides, preVertices, preIndices);

    fillVertexNormals(preVertices, preIndices, vertices, indices, triangleCount);
}