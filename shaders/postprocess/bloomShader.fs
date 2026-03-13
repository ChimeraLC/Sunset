#version 330 core
out vec4 fragColor;

in vec2 texPos;

uniform int stage;
uniform sampler2D screenTex;

uniform float weights[5] = float[](0.2270270270, 0.1945945946, 0.1216216216,
                                  0.0540540541, 0.0162162162);

void main()
{
    vec2 pixelSize = 5.0f / textureSize(screenTex, 0);
    vec3 outColor = texture(screenTex, texPos).rgb * weights[0];
    // TODO: Prevent bloom disappearing along edges
    vec2 offset = vec2(1 - stage, stage) * pixelSize; // Horizontal then vertical
    for (int i = 1; i < 5; i++)
    {
        outColor += texture(screenTex, texPos + offset * i).rgb * weights[i];
        outColor += texture(screenTex, texPos - offset * i).rgb * weights[i];
    }

    fragColor = vec4(outColor, 1);
} 