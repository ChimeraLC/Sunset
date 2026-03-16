// Renders a texture straight onto the screen
#version 330 core
layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 vertexNorm;

out vec2 texPos; 
uniform mat4 projection;
uniform mat4 view;

void main()
{
    texPos = vertexPos.xz / 4 + 0.5;
    gl_Position = projection * view * vec4(vertexPos, 1.0); 
}  