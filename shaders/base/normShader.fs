#version 330 core

out vec4 fragColor;

in vec3 normal;  

void main()
{
    vec3 norm = normalize(normal);
    fragColor = vec4(norm, 1.0);
}