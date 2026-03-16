#version 330 core
out vec4 fragColor;

in vec2 texPos;

uniform sampler2D lightraysTex;
uniform sampler2D skyTex;
uniform vec3 lightColor;
uniform vec3 baseColor;
uniform float time;
uniform float wind;

void main()
{    
    vec4 tex = texture(skyTex, texPos / 2 + vec2(time / wind, time / wind / 1.5));
    
    // Darker spots have higher density
    float density = 1 - tex.r + cos(time / 60) / 10;
    density = density * 2 - 1;
    density *= density;
    density *= 0.95;

    // Fade out with distance
    float disFromCenter = length(vec2(abs(texPos.x - 0.5), abs(texPos.y - 0.5)));
    if (disFromCenter > 0.25)
        density *= 2 - disFromCenter * 4;

    vec3 outColor = baseColor;
    vec2 screenPos = gl_FragCoord.xy / vec2(1920, 1080);
    
    float intensity = texture(lightraysTex, screenPos).r;
    intensity *= 4;
    outColor = (intensity * lightColor + outColor) / (intensity + 1);

    //fragColor = vec4(intensity * lightColor, 1);
    fragColor = vec4(outColor, density);
    
}