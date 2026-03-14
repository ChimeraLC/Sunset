// Final overall postprocess / combination shader
#version 330 core

out vec4 fragColor;

in vec2 texPos;

uniform sampler2D screenTex;
uniform sampler2D bloomTex;
uniform vec3 sunColor;

void main()
{
    vec3 screenColor = texture(screenTex, texPos).rgb;
    float bloomIntensity = texture(bloomTex, texPos).r;
    vec3 bloomColor = bloomIntensity * sunColor;
    vec3 outColor = bloomIntensity * sunColor + (1 - bloomIntensity) * screenColor;
    fragColor = vec4(outColor, 1.0);
}