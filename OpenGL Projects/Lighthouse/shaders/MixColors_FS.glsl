#version 330

// Input
in vec2 textureCoord;
in vec3 vColor;
in vec3 fragPos;

// Uniform properties
uniform sampler2D texture1;
uniform sampler2D texture2;
// TODO(student): Declare various other uniforms
uniform float uTime;
uniform ivec2 resolution;

// Output
layout(location = 0) out vec4 outColor;

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution;
    // uv *= 8;

    uv.x *= 16;
    uv.y *= 9;
    
    // vec2 translate = vec2(uTime * 0.8, 0.0);
    // uv += translate;
    // uv = fract(uv);
    
    // outColor = texture(texture1, uv);
    vec4 stripes = texture(texture1, uv);
    
    // outColor.xyz = mix(outColor.xyz, stripes.xyz, 0.2);
    outColor = stripes;
}
