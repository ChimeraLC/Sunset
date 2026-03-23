#version 330 core
out vec4 fragColor;

in vec3 TexCoords;

uniform sampler2D lightraysTex;
uniform samplerCube skyboxTex;

uniform vec2 screenSize;
uniform vec3 lightColor;
uniform bool occlusionRendering;

void main()
{    
    vec4 tex = texture(skyboxTex, TexCoords);
    if (occlusionRendering)
    {   
        if (tex.a < 0.1)
            discard;
        fragColor = vec4(0, 0, 0, 1);
    }
    else
    {   
        vec2 screenPos = gl_FragCoord.xy / screenSize;
        float intensity = texture(lightraysTex, screenPos).r;
        // Apply stronger lightrays to background
        if (tex.a < 0.5)
        {
            vec3 result = intensity * lightColor / 2 + tex.rgb;
            fragColor = vec4(result, 1.0);
        }
        else
        {
            vec3 result = intensity / 4 * lightColor + lightColor * tex.rgb;
            fragColor = vec4(result, 1.0);
        }
    }
}