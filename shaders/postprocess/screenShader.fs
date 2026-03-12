#version 330 core
out vec4 FragColor;

in vec2 texPos;

uniform sampler2D screenTex;

void main()
{
    FragColor = vec4(texture(screenTex, texPos).rgb, 1.0);
} 