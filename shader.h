#include <string>

#include <glm/glm.hpp>

using namespace std;
using namespace glm;

class Shader
{
    public:
        Shader();
        Shader(string vertexPath, string fragmentPath);
        
        bool isValid;

        void setActive() const;
        void deleteProgram() { glDeleteProgram(shaderProgram); }

        void setUniform(const string name, bool value) const;
        void setUniform(const string name, int value) const;
        void setUniform(const string name, float value) const;
        void setUniform(const string name, vec3 value) const;
        void setUniform(const string name, vec4 value) const;
        void setUniform(const string name, mat3 value) const;
        void setUniform(const string name, mat4 value) const;

    private:
        unsigned int shaderProgram;
};