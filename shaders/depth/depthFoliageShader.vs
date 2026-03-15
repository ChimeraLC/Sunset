#version 330 core
layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 vertexNorm;
layout (location = 2) in vec3 vertexOffset;

uniform mat4 model;
uniform mat4 lightView;

uniform float time;

void main()
{
    // This is kind of inefficient, since it never changes
    mat4 newModel = model;
    newModel[0][0] = cos(vertexOffset.z);
    newModel[2][0] = -sin(vertexOffset.z);
    newModel[0][2] = sin(vertexOffset.z);
    newModel[2][2] = cos(vertexOffset.z);
    newModel[3][0] = vertexOffset.x;
    newModel[3][2] = vertexOffset.y;

    gl_Position = lightView * newModel * vec4(vertexPos, 1.0);
    
    vec3 fragPos = vec3(newModel * vec4(vertexPos, 1.0));
    float wind = (cos(time + fragPos.x * 2 + sin(time + fragPos.y) * 3)) / 5
        * vertexPos.y;

    gl_Position.x += wind;
}