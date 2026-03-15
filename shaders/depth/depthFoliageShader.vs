#version 330 core
layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 vertexNorm;
layout (location = 2) in mat4 vertexOffset;

uniform mat4 model;
uniform mat4 lightView;

uniform float time;

void main()
{
    gl_Position = lightView * vertexOffset * vec4(vertexPos, 1.0);
    
    vec3 fragPos = vec3(vertexOffset * vec4(vertexPos, 1.0));
    float wind = (cos(time + fragPos.x * 2 + sin(time + fragPos.y) * 3)) / 5
        * vertexPos.y;

    gl_Position.x += wind;
}