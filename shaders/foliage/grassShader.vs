#version 330 core
layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 vertexNorm;
layout (location = 2) in vec3 vertexOffset;

out vec3 fragPos;
out float wind;
out vec3 fragLightPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normMatrix;
uniform mat4 lightView;
uniform vec3 lightDir;

uniform float time;

void main()
{
    // This is kind of inefficient, since it never changes
    mat4 newModel = model;
    newModel[0][0] = cos(vertexOffset.z);
    newModel[2][0] = -sin(vertexOffset.z);
    newModel[0][2] = sin(vertexOffset.z);
    newModel[2][2] = cos(vertexOffset.z);
    newModel[3][0] = vertexOffset.x;
    newModel[3][2] = vertexOffset.y;

    gl_Position = projection * view * newModel * vec4(vertexPos, 1.0);

    fragPos = vec3(newModel * vec4(vertexPos, 1.0));
    wind = (cos(time + fragPos.x * 2 + sin(time + fragPos.y) * 3)) / 5
        * vertexPos.y;
    
    // Since it's not back culled, if we're looking at if from the
    vec3 camView = vec3(view[2][0], view[2][1], -view[2][2]);
    // opposite direction light, it has to be flipped
    fragPos += sign(dot(camView, -lightDir)) * wind;

    // Doing perspective divide
    vec4 fragLightPrePos = lightView * vec4(fragPos, 1.0);
    fragLightPos = fragLightPrePos.xyz / fragLightPrePos.w;

    gl_Position.x += wind;
}