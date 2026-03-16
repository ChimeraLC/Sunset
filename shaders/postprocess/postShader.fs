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

    // Tone mapping
    const float gamma = 1.1;
    vec3 mapped = outColor / (outColor + vec3(1.0));
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    // Vignette

	mapped *= pow( 16 * texPos.x*(1-texPos.x) * texPos.y * (1.0-texPos.y), 0.1 );

    //fragColor = vec4(outColor, 1.0);
    fragColor = vec4(mapped, 1.0);
}