// Low down version of flat shader
#version 330 core

out vec4 fragColor;

in vec3 fragPos;
in vec3 fragLightPos;

uniform sampler2D shadowMap;
uniform sampler2D shadowMapRT;
uniform sampler2D lightraysTex;

uniform vec3 baseColor;
uniform bool renderOcclusion;
uniform vec3 lightColor;
uniform vec3 lightDir;


float shadowCalc(vec3 inFragLightPos)
{
    vec3 depthCoords = inFragLightPos * 0.5 + 0.5;
    float currentDepth = depthCoords.z;
    float shadowSmooth = 0.005f;

    if (currentDepth > 1.0)
        return 0.0;

    // Single sample
    float closestDepth = texture(shadowMap, depthCoords.xy).r;
    float inShadow = currentDepth - shadowSmooth > closestDepth  ? 1.0 : 0.0;

    closestDepth = texture(shadowMapRT, depthCoords.xy).r;
    float inShadowRT = currentDepth - shadowSmooth > closestDepth  ? 1.0 : 0.0;
    return max(inShadow, inShadowRT / 2);
}

void main()
{
    if (renderOcclusion)
        fragColor = vec4(0, 0, 0, 1);
    else
    {
        // TODO: SCREEN SIZE
        vec2 screenPos = gl_FragCoord.xy / vec2(1920, 1080);
        // God rays
        float intensity = texture(lightraysTex, screenPos).r;

        // Ambient lighting
        float ambientCoef = 0.25 * (0.5 + 8 * intensity);

        // Take same diffuse lighting as ground
        vec3 norm = vec3(0, 1, 0);

        vec3 lightTowards = normalize(-lightDir);
        float diff = max(dot(norm, lightTowards), 0.0);
        float diffuseCoef = diff;

        // Shadow coefficient
        float shadowCoef = shadowCalc(fragLightPos);

        vec3 result = (ambientCoef + (1.0 - shadowCoef) * diffuseCoef) * lightColor * baseColor;
        fragColor = vec4(result, 1.0);

    }
}