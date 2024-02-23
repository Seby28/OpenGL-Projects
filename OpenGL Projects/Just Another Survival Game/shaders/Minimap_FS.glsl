#version 430

// Input
layout(location = 0) in vec2 vTextureCoord;

// Uniform properties
uniform sampler2D uTexture;
uniform ivec2 uResolution;
uniform float uTime;
uniform bool uRave;

// Output
layout(location = 0) out vec4 outColor;

vec4 grayscale()
{
    vec4 color = texture(uTexture, vTextureCoord);
    float gray = 0.21 * color.r + 0.71 * color.g + 0.07 * color.b;

    return vec4(gray, gray, gray,  0);
}

vec4 negative()
{
    vec4 color = texture(uTexture, vTextureCoord);
    float R = 1 - color.r;
    float G = 1 - color.g;
    float B = 1 - color.b;

    return vec4(R, G, B,  0);
}

float circle(vec2 uv, float radius)
{
    vec2 dist = uv - vec2(0.5);
    return smoothstep(radius - (radius * 0.1), radius + (radius * 0.1), dot(dist, dist) * 10.0);
}

void main()
{
    vec2 uv = vTextureCoord;
    vec3 color = vec3(0.0);

    vec3 texColor = texture(uTexture, vTextureCoord).xyz;
    
    float factor = circle(uv, 2.0);
    color = factor * vec3(0.0, 1.0, 0.0) + (1.0 - factor) * texColor;

    outColor = vec4(texColor, 1.0);
    outColor = (!uRave) ? grayscale() : vec4(texColor, 0.5);

    // if (outColor.xyz == vec3(0.0))
    //     discard;
}
