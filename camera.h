#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

// Camera class that just has a position and a direction it's looking
class PositionalCamera
{
    public:
        PositionalCamera(vec3 position, vec3 worldUp = vec3(0.0f, 1.0f, 0.0f)) :
            position(position),
            worldUp(worldUp),
            target(vec3(0)), facing(vec3(0)), up(vec3(0))
        {}

        void SetPosition(vec3 newPosition) { position = newPosition; RecalcVectors(); }
        void SetTarget(vec3 newTarget) { target = newTarget; RecalcVectors(); }
    
        void RecalcVectors() {
            facing = normalize(target - position);
            vec3 right = normalize(cross(facing, worldUp));
            up = normalize(cross(right, facing));
        }

        mat4 GetLookAt()
        {
            return lookAt(position, position + facing, up);
        }

    private:
        vec3 position;
        vec3 worldUp;
        vec3 target;
        vec3 facing;
        vec3 up;
};