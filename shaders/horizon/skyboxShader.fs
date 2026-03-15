#version 330 core
out vec4 fragColor;

in vec3 TexCoords;

uniform samplerCube skyboxTex;
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
        fragColor = tex;
}