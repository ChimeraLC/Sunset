#version 330 core
layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 vertexNorm;

out vec3 fragPos;
out vec3 normal;
out vec3 fragLightPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normMatrix;
uniform mat4 lightView;

void main()
{
    gl_Position = projection * view * model * vec4(vertexPos, 1.0);
    fragPos = vec3(model * vec4(vertexPos, 1.0));
    normal = normMatrix * vertexNorm;

    // Doing perspective divide
    vec4 fragLightPrePos = lightView * vec4(fragPos, 1.0);
    fragLightPos = fragLightPrePos.xyz / fragLightPrePos.w;
}