#version 330 core
out vec4 fragColor;

in vec2 texPos;

uniform sampler2D screenTex;

void main()
{
    fragColor = vec4(texture(screenTex, texPos).rgb, 1.0);
} 