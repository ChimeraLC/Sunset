// Renders a texture straight onto the screen
#version 330 core
layout (location = 0) in vec2 vertexPos;
layout (location = 1) in vec2 inTexPos;

out vec2 texPos;

void main()
{
    texPos = inTexPos;
    gl_Position = vec4(vertexPos.x, vertexPos.y, 0.0, 1.0); 
}  