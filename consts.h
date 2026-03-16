#include <glm/glm.hpp>

#ifndef CONSTS_H
#define CONSTS_H
using namespace glm;

const float PI = 3.14159;
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

const vec3 VECTOR_ALIGN = vec3(1, 0, 0);
const vec3 WORLD_UP = vec3(0, 1, 0);

enum TIMES {
    DAY,
    DUSK,
    NIGHT
};

#endif // CONSTS_H