#include <glad/glad.h> 

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "shader.h"

using namespace std;

Shader::Shader(string vertexPath, string fragmentPath)
{
    string vertexString;
    string fragmentString;
    const char* vertexCode;
    const char* fragmentCode;
    ifstream vertexFile;
    ifstream fragmentFile;
    // Set exception bits
    vertexFile.exceptions (ifstream::badbit);
    fragmentFile.exceptions (ifstream::badbit);
    try 
    {
        // Open  and read files
        vertexFile.open(vertexPath);
        fragmentFile.open(fragmentPath);
        stringstream vShaderStream, fShaderStream;
        vShaderStream << vertexFile.rdbuf();
        fShaderStream << fragmentFile.rdbuf();	

        // Convert Streams to const chars
        vertexString = vShaderStream.str();
        fragmentString = fShaderStream.str();		
        
        vertexCode = vertexString.c_str();
        fragmentCode = fragmentString.c_str();	

        // Close file handlers
        vertexFile.close();
        fragmentFile.close();
        
    }
    catch (ifstream::failure& e)
    {
        cerr << "Failed to read shader files:\n " << e.what() << endl;
        isValid = false;
        return;
    }

    int success;
    char infoLog[512];

    // Compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexCode, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex compilation failed:\n" << infoLog << std::endl;
        isValid = false;
        return;
    }

    // Compile fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentCode, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment compilation failed:\n" << infoLog << std::endl;
        isValid = false;
        return;
    }
    
    // Link shaders and delete
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader linking failed:\n" << infoLog << std::endl;
        isValid = false;
        return;
    }

    isValid = true;
}

void Shader::setActive() const
{
    glUseProgram(shaderProgram);
}

void Shader::setUniform(const string name, bool value) const
{
    int uniformLocation = glGetUniformLocation(shaderProgram, name.c_str());
    glUniform1i(uniformLocation, (int)value);
}

void Shader::setUniform(const string name, int value) const
{
    int uniformLocation = glGetUniformLocation(shaderProgram, name.c_str());
    glUniform1i(uniformLocation, value);
}

void Shader::setUniform(const string name, float value) const
{
    int uniformLocation = glGetUniformLocation(shaderProgram, name.c_str());
    glUniform1f(uniformLocation, value);
}