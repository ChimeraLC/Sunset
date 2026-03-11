#include <glm/glm.hpp>

#include <vector>

enum ModelType {
	MODEL_DEFAULT = 0x01,
	MODEL_LIGHTSOURCE = 0x02,
};

struct ModelData {
	glm::vec3 color;
	glm::vec3 translation = glm::vec3(0);
	glm::vec4 rotation = glm::vec4(0); // Rotation angle stored in w
	unsigned int modelType = 0;
};

// Creates the vertices and indices for a model
unsigned int createModel(int index, std::vector<float>& vertices, std::vector<int>& indices,
	ModelData& modelData, int& triangleCount);

// Based on triangle vertices + indices setup without normals and with repeats, creates full vectors
void fillVertexNormals(std::vector<float> const& preVertices, 
	std::vector<int> const& preIndices,
	std::vector<float>& vertices, std::vector<int>& indices,
	int& triangleCount);

void createModelGround(std::vector<float>& vertices, std::vector<int>& indices, 
	ModelData& modelData, int& triangleCount);
void createModelSun(std::vector<float>& vertices, std::vector<int>& indices, 
	ModelData& modelData, int& triangleCount);
void createModelTrunk(std::vector<float>& vertices, std::vector<int>& indices, 
	ModelData& modelData, int& triangleCount, glm::vec3 offset);

glm::vec3 getNormal(const float* point1, const float* point2, const float* point3);