#version 330 core
out vec4 FragColor;

in vec2 texPos;
in vec2 screenPos;

uniform sampler2D screenTex;
uniform vec3 lightScreenPos;

void main()
{
    int sampleCount = 100;
    vec2 sampleDir = (lightScreenPos.xy - screenPos) / sampleCount;
    vec2 samplePos = screenPos;
    vec3 color = vec3(0);
    float density = 1.0f;
    for (int i = 0; i < sampleCount; i++)
    {
        samplePos += sampleDir;
        color += texture(screenTex, samplePos).rgb * density / sampleCount;
        density *= 0.99f;
    }
    FragColor = vec4(color, 1);
} 