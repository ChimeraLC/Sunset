#version 330 core

out vec4 fragColor;

uniform bool isLight;

void main()
{
    if (isLight)
        fragColor = vec4(1.0, 1.0, 1.0, 1.0);
    else
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
}