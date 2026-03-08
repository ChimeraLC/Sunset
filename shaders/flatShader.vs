#version 330 core
layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 vertexNorm;

out vec3 fragPos;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normMatrix;

void main()
{
    gl_Position = projection * view * model * vec4(vertexPos, 1.0);
    fragPos = vec3(model * vec4(vertexPos, 1.0));
    normal = normMatrix * vertexNorm;
}