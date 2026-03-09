#include <glm/glm.hpp>

#include <vector>

// Creates the vertices and indices for a model
unsigned int createModel(int index, std::vector<float>& vertices, std::vector<int>& indices, int& triangleCount);

// Based on triangle vertices + indices setup without normals and with repeats, creates full vectors
void fillVertexNormals(std::vector<float> const& preVertices, 
        std::vector<int> const& preIndices,
        std::vector<float>& vertices, std::vector<int>& indices,
        int triangleCount);

void createModelGround(std::vector<float>& vertices, std::vector<int>& indices, int& triangleCount);
void createModelTrunk(std::vector<float>& vertices, std::vector<int>& indices, int& triangleCount);

glm::vec3 getNormal(const float* point1, const float* point2, const float* point3);