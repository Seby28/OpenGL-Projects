#version 330

// Input
in vec2 vTextureCoord;

// Uniform properties
uniform sampler2D texture1;

// Output
layout(location = 0) out vec4 outColor;

void main()
{
    // vec2 uv = vTextureCoord;
    // uv *= 16;
    // 
    // vec2 translate = vec2(uTime * 0.1, 0.0);
    // uv += translate;
    // 
    // outColor = texture(texture1, uv);
    // 
    // vec2 modulated_coords = (vec2(sin(worldPosition.x * 10)) + 1) * 0.5;
    // modulated_coords += translate;
    // modulated_coords = fract(modulated_coords);
    //outColor = vec4(modulated_coords.x, modulated_coords.x, modulated_coords.x, 1);

    //vec3 material = texture(texture1, vTextureCoord).xyz;

    //vec3 illumination = ComputePointLightSourcesIllumination(material) + ComputeAmbientComponent(material);

    outColor = texture(texture1, vTextureCoord);
}
