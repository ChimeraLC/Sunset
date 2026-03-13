#version 330 core
out vec4 fragColor;

in vec2 texPos;

uniform sampler2D screenTex;
uniform vec3 lightScreenPos;

void main()
{
    int sampleCount = 100;
    vec2 sampleDir = (lightScreenPos.xy - texPos) / sampleCount;
    vec2 samplePos = texPos;
    vec3 color = vec3(0);
    float density = 1.0f;
    for (int i = 0; i < sampleCount; i++)
    {
        samplePos += sampleDir;
        color += texture(screenTex, samplePos).rgb * density / sampleCount;
        density *= 0.99f;
    }
    fragColor = vec4(color, 1);
} 