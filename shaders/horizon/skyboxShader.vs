#version 330 core
layout (location = 0) in vec3 vertexPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = vertexPos;
    gl_Position = projection * view * vec4(vertexPos, 1.0);
}  