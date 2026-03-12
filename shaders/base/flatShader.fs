#version 330 core

out vec4 fragColor;

in vec3 normal;  
in vec3 fragPos;  
in vec3 fragLightPos;

uniform sampler2D shadowMap;
uniform sampler2D lightraysTex;

uniform vec3 baseColor;
uniform vec3 lightColor;
uniform vec3 lightDir;
uniform vec3 lightScreenPos;
uniform vec3 camPos;

// TODO: Fix aliasing on oblique surfaces
float shadowCalc(vec3 inFragLightPos, vec3 norm)
{
    vec3 depthCoords = inFragLightPos * 0.5 + 0.5;
    float currentDepth = depthCoords.z;
    float shadowSmooth = 0.005f;

    if (currentDepth > 1.0)
        return 0.0;

    // Bilinear shadow sampling    
    vec2 texelSize = 1.0f / textureSize(shadowMap, 0);
    vec2 scaledCoords = depthCoords.xy * textureSize(shadowMap, 0);
    float xRatio = scaledCoords.x - floor(scaledCoords.x) - 0.5;
    float yRatio = scaledCoords.y - floor(scaledCoords.y) - 0.5;

    float inShadow = 0;

    for (int i = 0; i < 4; i++)
    {        
        int xSlice = i % 2;
        int ySlice = i / 2;

        vec2 testPos = depthCoords.xy + 
            vec2(xSlice * sign(xRatio), ySlice * sign(yRatio)) * texelSize;

        // Compare closest depth with current depth
        float closestDepth = texture(shadowMap, testPos).r;
        inShadow += (currentDepth - shadowSmooth > closestDepth  ? 1.0 : 0.0)
            * (xSlice > 0 ? abs(xRatio) : 1 - abs(xRatio))
            * (ySlice > 0 ? abs(yRatio) : 1 - abs(yRatio));    
    }
    return inShadow;
}

void main()
{
    vec2 screenPos = gl_FragCoord.xy / vec2(1920, 1080);

    // God rays
    float intensity = texture(lightraysTex, screenPos).r;

    // Ambient lighting
    float ambientCoef = 0.25 * (0.5 + 4 * intensity);
    
    // Diffuse lighting TODO: Assert always pre-normalized if not scaling? reduce computation
    vec3 norm = normalize(normal);
    //vec3 lightDir = normalize(lightPos - fragPos);
    
    vec3 lightTowards = normalize(-lightDir);
    float diff = max(dot(norm, lightTowards), 0.0);
    float diffuseCoef = diff;

    // Specular lighting
    float specStrength = 0.0; // TODO: control externally; maybe pass in a vec3
    float specCoef = 0.0;
    if (specStrength > 0)
    {
        vec3 camDir = normalize(camPos - fragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(camDir, reflectDir), 0.0), 32);
        specCoef = specStrength * spec;  
    }  

    // Check if in shadow
    float shadowCoef = shadowCalc(fragLightPos, norm);
    
    vec3 result = (ambientCoef + (1.0 - shadowCoef) * (diffuseCoef + specCoef)) * lightColor * baseColor;
    fragColor = vec4(result, 1.0);
}