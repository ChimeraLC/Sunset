// Renders a texture straight onto the screen
#version 330 core
layout (location = 0) in vec2 vertexPos;

uniform vec2 offset;
out vec2 texPos;

void main()
{
    gl_Position = vec4(vertexPos.x / 30 + offset.x, vertexPos.y / 16.25 + offset.y, 0.0, 1.0); 
}  