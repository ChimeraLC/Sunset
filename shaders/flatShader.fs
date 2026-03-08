#version 330 core

out vec4 fragColor;

in vec3 normal;  
in vec3 fragPos;  
  
uniform vec3 baseColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 camPos;

void main()
{
    // Ambient lighting
    float ambientCoef = 0.25;
    
    // Diffuse lighting TODO: Assert always pre-normalized if not scaling? reduce computation
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
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

    vec3 result = (ambientCoef + diffuseCoef + specCoef) * lightColor * baseColor;
    fragColor = vec4(result, 1.0);
}