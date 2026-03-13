#version 330 core
out vec4 fragColor;

in vec2 texPos;

void main()
{
    vec3 backgroundColorHigh = vec3(0.3, 0.5f, 0.6f);
    vec3 backgroundColorLow = vec3(0.75, 0.4, 0.4);
    fragColor = vec4(texPos.y / 1.5 * backgroundColorHigh + (1 - texPos.y / 1.5) * backgroundColorLow
    , 1.0f);
} 