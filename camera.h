#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "consts.h"

using namespace glm;

class Camera {
    public: 
        virtual mat4 GetLookAt() 
        {
            return lookAt(position, position + facing, up);
        }

        virtual void ProcessInput(GLFWwindow* window, float deltaTime) { 
            (void) window; (void) deltaTime; }
        virtual void ProcessMouse(float xOffset, float yOffset) { 
            (void) xOffset; (void) yOffset; }

    protected:
        vec3 position;
        vec3 worldUp;
        vec3 facing;
        vec3 up;
};

// Camera class that just has a position and a direction it's looking
class FixedCamera : public Camera
{
    public:
        FixedCamera(vec3 _position, vec3 _worldUp = vec3(0.0f, 1.0f, 0.0f))
        {
            position = _position;
            worldUp = _worldUp;
            facing = vec3(0);
            up = vec3(0);
            target = vec3(0);
            cameraTime = 0;
        }

        virtual void ProcessInput(GLFWwindow* window, float deltaTime) {
            (void) window;
            cameraTime += deltaTime / 3;
            SetPosition(vec3(3 * cos(cameraTime), 2, 3 * sin(cameraTime)));
        }

        void SetPosition(vec3 newPosition) { position = newPosition; RecalcVectors(); }
        void SetTarget(vec3 newTarget) { target = newTarget; RecalcVectors(); }

    protected:
        void RecalcVectors() {
            facing = normalize(target - position);
            vec3 right = normalize(cross(facing, worldUp));
            up = normalize(cross(right, facing));
        }

    private:
        vec3 target;
        float cameraTime;
};

const float MOVE_SPEED = 1.0f;
const float LOOK_SPEED = 1e-3;

// Camera that is controlled by keys
class FreeCamera : public Camera
{
    public:
        FreeCamera(vec3 _position, float _yaw, float _pitch, vec3 _worldUp = vec3(0.0f, 1.0f, 0.0f))
        {
            position = _position;
            worldUp = _worldUp;
            facing = vec3(0);
            up = vec3(0);

            yaw = _yaw;
            pitch = _pitch;
        }

        virtual void ProcessInput(GLFWwindow* window, float deltaTime) {
            vec3 movement2D = vec3(
                (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) - (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS),
                0,
                (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) - (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            );

            float movementVert = 
                (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) - (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS);
            
                // Since it's 2d, we'll only ever need to normalize when x != 0
            if (movement2D.x != 0)
                movement2D = normalize(movement2D);
            
            position += MOVE_SPEED * deltaTime * 
                (movement2D.x * facing + movement2D.z * right);
            position.y += MOVE_SPEED * deltaTime * movementVert;
            
            RecalcVectors();
        }

        virtual void ProcessMouse(float xOffset, float yOffset) { 
            yaw += xOffset * LOOK_SPEED;
            pitch += yOffset * LOOK_SPEED;

            pitch = glm::max(glm::min(pitch, PI / 2), -PI / 2);
        }

    protected:
        void RecalcVectors() {
            facing = normalize(vec3(
                cos(yaw) * cos(pitch),
                sin(pitch),
                sin(yaw) * cos(pitch)
            ));
            right = normalize(cross(facing, worldUp));
            up    = normalize(cross(right, facing));
        }


    private:
        vec3 right;
    
        float yaw;
        float pitch;
};
