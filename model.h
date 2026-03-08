#include <vector>

// Creates the vertices and indices for a model
unsigned int createModel(int index, std::vector<float>& vertices, std::vector<int>& indices, int& triangleCount);

void createModelGround(std::vector<float>& vertices, std::vector<int>& indices, int& triangleCount);
void createModelTrunk(std::vector<float>& vertices, std::vector<int>& indices, int& triangleCount);