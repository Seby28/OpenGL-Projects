#version 430

// Input
layout(location = 0) in vec2 vTextureCoord;

// Uniform properties
uniform sampler2D uTexture;
uniform ivec2 uResolution;
uniform float uTime;
uniform float vignette;

// Output
layout(location = 0) out vec4 outColor;

void main()
{
    vec2 uv = vTextureCoord;
    vec3 color = vec3(0.0);

    float pct = distance(uv, vec2(0.5));

    vec3 texColor = texture(uTexture, vTextureCoord).xyz;

    vec3 tex = mix(texColor, vec3(1.0, 0.0, 0.0), vignette);

    color = vec3(pct - 0.3);
    
    outColor = vec4(color * tex + (1.0 - color) * texColor, 1.0);
}
