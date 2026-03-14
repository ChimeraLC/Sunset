#include <string>

#include <glm/glm.hpp>

using namespace std;
using namespace glm;

class Shader
{
    public:
        Shader();
        Shader(string vertexPath, string fragmentPath, string _shaderName);

        static bool shadersValid;

        void setActive() const;
        void deleteProgram() { 
            for (Shader* shader : shaders)
                glDeleteProgram(shader->shaderProgram); 
        }

        void setUniform(const string name, bool value) const;
        void setUniform(const string name, int value) const;
        void setUniform(const string name, float value) const;
        void setUniform(const string name, vec3 value) const;
        void setUniform(const string name, vec4 value) const;
        void setUniform(const string name, mat3 value) const;
        void setUniform(const string name, mat4 value) const;

    private:
        static vector<Shader*> shaders;
        unsigned int shaderProgram;
        string shaderName;
};