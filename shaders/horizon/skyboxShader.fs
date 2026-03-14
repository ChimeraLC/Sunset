#version 330 core
out vec4 fragColor;

in vec3 TexCoords;

uniform samplerCube skyboxTex;

void main()
{    
    fragColor = texture(skyboxTex, TexCoords);
}