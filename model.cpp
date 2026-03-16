#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <tuple>
#include <random>
#include <iostream>

#include "consts.h"
#include "texture.h"

#include "model.h"

using namespace glm;
using namespace std;

unsigned int createModel(int index, vector<float>& vertices, vector<int>& indices, 
    ModelData& modelData, int& triangleCount, std::vector<glm::mat4>& instanceData,
    Image designTextures[], int designTextureCount) {
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
            createModelTrunk(vertices, indices, modelData, triangleCount,
                designTextureCount >= 1 ? designTextures[0] : Image());
            break;
        case MODELTYPE_GRASS:
            createModelGrass(vertices, indices, modelData, triangleCount, instanceData,
                designTextureCount >= 2 ? designTextures[1] : Image());
            break;
        case MODELTYPE_LEAVES:
            createModelLeaves(vertices, indices, modelData, triangleCount, instanceData);
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

void displayVec3(vec3 vector)
{
    cout << round(vector.x * 1000) / 1000 << ", " << round(vector.y * 1000) / 1000 
        << ", " << round(vector.z * 1000) / 1000 << endl;
}


void displayValues(vector<float>& preVertices, vector<int>& preIndices)
{
    std::cout << "\nVertices:" << endl;
    for (int i = 0; i < (int) preVertices.size() / 3; i++)
    {
        displayVec3( vec3(preVertices[i * 3], preVertices[i * 3 + 1], preVertices[i * 3 + 2]));
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
    ModelData& modelData, int& triangleCount, std::vector<glm::mat4>& instanceData,
    Image designTexture) {
        
    modelData.modelType |= MODEL_FOLIAGE;
    modelData.translation = vec3(0, 0, 0.0);

    vector<float> preVertices = {
        0.025, 0, 0,
        -0.025, 0, 0,
        0, 0.16, 0,
    };

    vector<int> preIndices = {
            0, 1, 2
    };

    fillVertexNormals(preVertices, preIndices, vertices, indices, triangleCount);

    modelData.color = vec3(0.0f, 0.5f, 0.0f);

    // TODO: Why does this crash sometimes?
    if (designTexture.data != nullptr)
    {
        int count = 0;
        
        int extent = 1;
        int frequency = 30;

        // If a spot is colored in, add grass there
        for (int row = -frequency; row <= frequency; row++)
        {
            for (int col = -frequency; col <= frequency; col++)
            {
                // No grass near tree base
                if (abs(col) + abs(row) <= 2)
                    continue;

                vec2 testPos = vec2(row + frequency, col + frequency) / (2.f * (frequency + 1))
                    * vec2(designTexture.height, designTexture.width);
                
                if (getColor(designTexture, testPos.x, testPos.y).x < 0.5
                    || getColor(designTexture, designTexture.height - testPos.x, 
                        designTexture.width - testPos.y).x < 0.5)
                {
                    mat4 model = mat4(1.0f);
                    model = translate(model, glm::vec3((float) col / frequency * extent, 
                        0, (float) row / frequency * extent));
                    model = scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
                    model = rotate(model, 17.0f * (row + col), WORLD_UP);
                    instanceData.push_back(model);

                    count++;
                }
            }
        }
        // Day night dependent on second texture
        if (count > frequency * frequency * 0.7f)
            setTime(NIGHT);

        modelData.instanceCount = count;

    }
    else // Backup scene
    {
        int count = 10;
        modelData.instanceCount = count * count;
        for (unsigned int j = 0; j < modelData.instanceCount; j++)
        {
            mat4 model = mat4(1.0f);
            model = translate(model, glm::vec3(- (float) count / 20 + (float)(j / count) / count, 
                0, -  (float) count / 20 + (float)(j % count) / count));
            model = scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
            model = rotate(model, 17.0f * j, WORLD_UP);
            instanceData.push_back(model);
        }
    }
}

float leafiness = 150.f;
float maxHeight = 1.0f;
vector<mat4> leaves;
void createModelLeaves(vector<float>& vertices, vector<int>& indices, 
    ModelData& modelData, int& triangleCount, std::vector<glm::mat4>& instanceData) {
        
    modelData.modelType |= MODEL_FOLIAGE;
    modelData.translation = vec3(0, 0, 0.0);

    vector<float> preVertices = {
        0, 0, 0,
        -0.04, 0.01, 0,
        0.04, 0.01, 0,
        0, 0.09, 0
    };

    vector<int> preIndices = {
            0, 1, 3,
            0, 3, 2
    };

    fillVertexNormals(preVertices, preIndices, vertices, indices, triangleCount);

    modelData.color = vec3(0.0f, 0.6f, 0.0f);
    modelData.instanceCount = leaves.size();
    instanceData = leaves; // TODO: Is this safe?
}

// Start and end denote areas leaf could be
void createLeaf(vec3 startPos, vec3 endPos, vec3 pointing, float probScale = 1.0, bool maxOne = false)
{
    for (int i = 1; i < randFloat() * (1 + leafiness * probScale); i++)
    {
        mat4 model = mat4(1);

        // Translate
        model = translate(model, startPos + randFloat() * (endPos - startPos));

        pointing = normalize(pointing);
        float dotVal = dot(WORLD_UP, pointing);
        clamp(dotVal,-1.0f,1.0f);

        // Rotate in the general direction
        vec3 axis = cross(WORLD_UP, pointing);
        model = rotate(model, randFloat() * PI, WORLD_UP); // I don't know the math TT
        model = rotate(model, glm::acos(dotVal),axis);
        leaves.push_back(model);
    
        if (maxOne)
            return;
    }
}
// MARK: TREE

// Creates a tree ring of points; returns start point of the ring
// Currently is not orriented any way
int createTreeRing(vec3 ringCenter, vec3 inDirection, float radius, int sides, vector<float>& vertices, int overrideIndex = -1)
{
    // First index is always aligned along vec3(1, 0, 0)
    vec3 right = normalize(cross(inDirection, VECTOR_ALIGN));
    vec3 forward = -normalize(cross(inDirection, right));

    int startIndex = vertices.size() / 3;

    float angle = 2 * PI / sides; // Wind clockwise
    for (int i = 0; i < sides; i++)
    {
        vec3 offset = radius * (cos(angle * i) * forward + sin(angle * i) * right);
        vec3 newPoint = ringCenter + offset;
        if (overrideIndex == -1)
            pushVertex(vertices, newPoint);
        else
        {
            vertices[(overrideIndex + i) * 3] = newPoint.x;
            vertices[(overrideIndex + i) * 3 + 1] = newPoint.y;
            vertices[(overrideIndex + i) * 3 + 2] = newPoint.z;
        }
    }

    return startIndex;
}

// Adds the necessary triangles to connect two tree rings
// Currently assumes sides and orrentation is the same
void connectTreeRings(vec3 startPoint, vec3 endPoint, int indexA, int indexB, int sides,
        vector<float>& vertices, vector<int>& indices)
{
    float branchLength = length(endPoint - startPoint);
    for (int i = 0; i < sides; i++)
    {
        int left = i;
        int right = (i + 1) % sides;
        pushIndices(indices, indexA + left, indexA + right, indexB + left);
        pushIndices(indices, indexA + right, indexB + right, indexB + left);
        
        // Add leaves along edge
        vec3 baseVertex = getVertex(vertices, indexA + left);
        vec3 endVertex = getVertex(vertices, indexB + left);
        vec3 fromCenter = baseVertex - startPoint;
        vec3 toEnd = endVertex - baseVertex;
        vec3 out = cross(toEnd, cross(fromCenter, baseVertex));
        // More leaves near the top, equal amounts per branch lengths, and irrespective of sidecount
        float heightFactor = ((baseVertex + endVertex) / 2.f).y / maxHeight;
        heightFactor = clamp(2 * heightFactor - 0.5f, 0.f, 1.f);
        createLeaf(baseVertex, endVertex, out, heightFactor * branchLength / sides);
    }
}

//Ends off a tree branch; should work the same as connectTreeRings with sidesB = 1
// Index A is a ring, index B is a single point
void capBranch(vec3 branchStart, vec3 branchEnd, int indexA, int sides,
        vector<float>&vertices, vector<int>& indices)
{
    int startIndex = vertices.size() / 3;

    pushVertex(vertices, branchEnd);

    float branchLength = length(branchStart - branchEnd);

    for (int i = 0; i < sides; i++)
    {
        int left = i;
        int right = (i + 1) % sides;
        pushIndices(indices, indexA + left, indexA + right, startIndex);

        // Add leaves along edge
        vec3 baseVertex = getVertex(vertices, indexA + left);
        vec3 fromCenter = baseVertex - branchStart;
        vec3 toEnd = branchEnd - baseVertex;
        vec3 out = cross(toEnd, cross(fromCenter, baseVertex));
        // More leaves near the top, equal amounts per branch lengths, and irrespective of sidecount
        float heightFactor = ((baseVertex + branchEnd) / 2.f).y / maxHeight;
        heightFactor = clamp(2 * heightFactor - 0.3f, 0.f, 1.f);
        createLeaf(baseVertex, branchEnd, out, heightFactor * branchLength / sides);
    }
    // Create leaves at ends
    createLeaf(branchEnd, branchEnd, branchEnd-branchStart, branchEnd.y / maxHeight, true);
}

//Splits a ring into two branch points (split direction is perpendicular to line split on)
void splitTreeRing(vec3 startPos, int index, vec3 splitDirection, int& outIndexA, int& outIndexB, int sides,
    vector<float>&vertices, vector<int>& indices)
{
    // These values are always a given
    int startIndex = vertices.size() / 3;
    
    outIndexA = startIndex;
    outIndexB = startIndex + sides;
    
    // Find split index point
    vec3 center = startPos;

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
    int offset = mostAligned + closest;
    // Forward side partition
    for (int i = 0; i < sides; i++)
    {   
        int trueIndex = (offset + i) % sides;
        if (trueIndex >= lowNew && trueIndex <= highNew)
            pushVertex(vertices, newPoints[trueIndex - lowNew]);
        else
            pushVertex(vertices, getVertex(vertices, index + (closest + trueIndex) % sides));
    }

    offset = (mostAligned + closest + (sides / 2)) % sides;
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

// TODO: Creating a bunch of new vectors cannot be efficient, although since the count is so low
// it might be fine
void splitPoints(vector<vec2> inPoints, vector<vec2>& outPointsA, vector<vec2>& outPointsB)
{
    float farLeft = FLT_MAX;
    float farRight = -FLT_MAX;
    float center = 0;
    for (unsigned int i = 0; i < inPoints.size(); i++)
    {
        farLeft = std::min(inPoints[i].x, farLeft);
        farRight = std::max(inPoints[i].x, farRight);
        center += inPoints[i].x;
    }
    center = center / inPoints.size();

    // Split left and right of center, close to center are random
    float range = farRight - farLeft;
    float tossupLeft = farLeft + range * 0.5;
    float tossupRight = farLeft + range * 0.5;

    for (vec2 point : inPoints)
    {
        if (point.x < tossupLeft)
        {
            outPointsA.push_back(point);
        }
        else if (point.x > tossupRight)
        {
            outPointsB.push_back(point);
        }
        else
        {
            float prop = (point.x - tossupLeft) / (range * 0.1f);
            if (randFloat() < prop)
                outPointsB.push_back(point);
            else
                outPointsA.push_back(point);
        }
    }

    // TODO: Delete inPoints when done?
}

// Generates a new outgoing branch direction based on incoming direction
vec3 treeNewDirection(vec3 inDirection, vec3 startPoint, vec2 endPoint)
{
    vec3 newPoint = vec3(endPoint.x, endPoint.y, startPoint.z);
    float branchLength = length(newPoint - startPoint);

    float bump = randFloat() - 0.5f;
    newPoint.z += (inDirection.z / 2 + bump) * branchLength;
    return newPoint;
}

const int sides = 8;
void createModelSubtree(vec3 startPoint, vec3 inDirection, int startIndex, vector<vec2> inPoints, float radius, vector<float>& preVertices, vector<int>& preIndices,
    bool forceUnsplit = false, bool justForced = false)
{
    inDirection = normalize(inDirection);
    if (inPoints.size() == 0) // Cap off
    {
        vec3 newPoint = treeNewDirection(inDirection, startPoint, startPoint + 
                inDirection * (0.1f + randFloat() * 0.1f) + vec3(0, randFloat() * 0.05f, 0));
        capBranch(startPoint, newPoint, startIndex, sides, preVertices, preIndices);
    }
    else if (inPoints.size() == 1 || forceUnsplit)
    {
        vec2 inPoint = inPoints[inPoints.size() - 1];

        vec3 newPoint = treeNewDirection(inDirection, startPoint, inPoint);

        vec3 newInDirection = inDirection + normalize(newPoint - startPoint);
        newInDirection = normalize(newInDirection);

        float branchLength = length(newPoint - startPoint);
        radius *= pow(0.6f + randFloat() * 0.1f, branchLength);

        if (startIndex != 0)
            createTreeRing(startPoint, newInDirection, radius, sides, preVertices, startIndex);

        int index = createTreeRing(newPoint, newPoint - startPoint,
        radius, sides, preVertices);
        connectTreeRings(startPoint, newPoint, startIndex, index, sides, preVertices, preIndices);

        inPoints.pop_back();
        createModelSubtree(newPoint, newPoint-startPoint, index, inPoints, radius, preVertices, preIndices, false, forceUnsplit);
        // TODO: Decrease size with time
    }
    else
    {
        vector<vec2> leftPoints;
        vector<vec2> rightPoints;
        splitPoints(inPoints, leftPoints, rightPoints);             // Can't unsplit twice in a row
        if (leftPoints.size() == 0 || rightPoints.size() == 0 || (inPoints.size() > 3 && !justForced && randFloat() < 0.4f))
        {
            // Do stuff here
            createModelSubtree(startPoint, inDirection, startIndex, inPoints, radius, preVertices, preIndices, true);
        }
        else
        {
            vec2 rightPoint = rightPoints[rightPoints.size() - 1];
            vec2 leftPoint = leftPoints[leftPoints.size() - 1];
            vec3 newPointRight = treeNewDirection(inDirection, startPoint, rightPoint);
            vec3 newPointLeft = treeNewDirection(inDirection, startPoint, leftPoint);

            // Checking that they're not too overlapping
            vec3 leftDirection = normalize(newPointLeft - startPoint);
            vec3 rightDirection = normalize(newPointRight - startPoint);
            if (dot(leftDirection, rightDirection) > 0.95f)
            {
                createModelSubtree(startPoint, inDirection, startIndex, inPoints, radius, preVertices, preIndices, true);
            }
            else
            {
            vec3 splitDirection = newPointLeft - newPointRight; // TODO: Calculate this better once z != 0
            
            if (normalize(newPointRight - startPoint).x > normalize(newPointLeft - startPoint).x)
                splitDirection = -splitDirection;

            // Regenerate ring based on outgoing directions
            vec3 newInDirection = inDirection + normalize(newPointRight + newPointLeft - 2.0f * startPoint);
            newInDirection = normalize(newInDirection);
            createTreeRing(startPoint, newInDirection, radius, sides, preVertices, startIndex);

            // Split base
            int indexSplitL, indexSplitR;
            splitTreeRing(startPoint, startIndex, splitDirection, indexSplitR, indexSplitL, sides, preVertices, preIndices);

            // Create new branches
            radius *= 0.6f; // Radius reduced significantly on branching
            float branchLengthLeft = length(newPointLeft - startPoint);
            float branchLengthRight = length(newPointRight - startPoint);
            float radiusLeft = radius * pow(0.6f + randFloat() * 0.15f, branchLengthLeft);
            float radiusRight = radius * pow(0.6f + randFloat() * 0.15f, branchLengthRight);

            int indexL = createTreeRing(newPointLeft, newPointLeft-startPoint, 
                    radiusLeft, sides, preVertices);
            int indexR = createTreeRing(newPointRight, newPointRight-startPoint, 
                    radiusRight, sides, preVertices);
            connectTreeRings(startPoint, newPointLeft, indexSplitL, indexL, sides, preVertices, preIndices);
            connectTreeRings(startPoint, newPointRight, indexSplitR, indexR, sides, preVertices, preIndices);

            leftPoints.pop_back();
            rightPoints.pop_back();
            createModelSubtree(newPointLeft, newPointLeft-startPoint, indexL, leftPoints, radiusLeft, preVertices, preIndices);
            createModelSubtree(newPointRight, newPointRight-startPoint, indexR, rightPoints, radiusRight, preVertices, preIndices);
            }
        }
    }
}

// Starting at a position on the image, finds first image edge
vec2 sampleRow(Image image, vec2 startPos, bool goingRight, bool& found, float sampleRate = 0.05f)
{
    int col = getColor(image, startPos.y * image.height, startPos.x * image.width).r;
    for (int i = 1; i < 1 / sampleRate; i++)
    {
        startPos.x += (goingRight ? 1 : -1) * sampleRate;
        if (startPos.x > 1) startPos.x -= 1;
        if (startPos.x < 0) startPos.x += 1;

        if (getColor(image, startPos.y * image.height, startPos.x * image.width).r != col)
        {
            found = true;
            return startPos;
        }
    }
    found = false;
    return vec2(0);
}

void createModelTrunk(vector<float>& vertices, vector<int>& indices, 
    ModelData& modelData, int& triangleCount, Image designTexture) {

    modelData.modelType |= MODEL_DEFAULT;
    modelData.color = vec3(0.6f, 0.3f, 0.0f);
    modelData.translation = vec3(0.0f);
    

    if (sides % 2 == 1)
        cerr << "sides cannot be odd" << endl;

    vector<float> preVertices;
    vector<int> preIndices;

    // Input vector of points generated based on input texture
    // Ordered from highest to lowest. z will be random~ish
    vector<vec2> inputPoints;
    bool everFoundPoint = false;
    float stepSize = 0.025f;
    if (designTexture.data != nullptr)
    {
        for (float i = 0.9f; i >= 0.025f; i -= stepSize)
        {
            bool found = false;
            vec2 foundPoint = sampleRow(designTexture, vec2(randFloat(), i), 
                randFloat() > 0.5f ? true : false,
                found);

            if (found)
            {
                everFoundPoint = true;
                vec2 newPoint = foundPoint - vec2(0.5f, 0);
                newPoint.x = sign(newPoint.x) * glm::min(i * 1.f, abs(newPoint.x));
                inputPoints.push_back(newPoint);
            }
            if (everFoundPoint)
            {
                stepSize += 0.005f;
            }
        }
    }
    if (inputPoints.size() == 0)
    {
        inputPoints = { vec2(0.2, 0.7), vec2(-0.1, 0.7), vec2(0.25, 0.65), vec2(-0.3, 0.65), vec2(-0.2, 0.6), vec2(0.2, 0.6), vec2(0, 0.3)};
    }
    
    
    maxHeight = inputPoints[0].y;

    // Sanity check
    for (unsigned int i = 1; i < inputPoints.size(); i++)
    {
        if (inputPoints[i].y > inputPoints[i-1].y)
        {
            cerr << "tree input points is misordered" << endl;
        }
    }

    int startIndex = createTreeRing(vec3(0), vec3(0, 1, 0), 0.125f, sides, preVertices);
    createModelSubtree(vec3(0), vec3(0, 1, 0), startIndex, inputPoints, 0.125f, preVertices, preIndices, true);

    fillVertexNormals(preVertices, preIndices, vertices, indices, triangleCount);
}