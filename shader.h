#include <string>
using namespace std;

class Shader
{
    public:
        Shader(string vertexPath, string fragmentPath);
        
        bool isValid;

        void setActive() const;
        void deleteProgram() { glDeleteProgram(shaderProgram); }

        void setUniform(const string name, bool value) const;
        void setUniform(const string name, int value) const;
        void setUniform(const string name, float value) const;

    private:
        unsigned int shaderProgram;
};