#version 330

// Input
in vec2 textureCoord;

uniform float time;

// Output
layout(location = 0) out vec4 outColor;

float circle(vec2 uv, float radius)
{
    vec2 dist = uv - vec2(0.5);
    return smoothstep(radius - (radius * 0.01), radius + (radius * 0.01), dot(dist, dist) * 10.0);
}

void main()
{   
    vec2 uv = textureCoord;
    vec3 color = vec3(0.0);

    // if (((uv.x - 0.5) * (uv.x - 0.5) + (uv.y - 0.5) * (uv.y - 0.5)) < abs(sin(uTime * 4.0)) / 10.0)
    // {
    //     outColor = vec4(vec3(0.0), 1.0);
    // }
    // else
    // {
    //     outColor = vec4(1.0, 1.0, 0.0, 1.0);
    // }
    
    float factor = circle(uv, abs(sin(time * 5.0)));
    color = factor * vec3(1.0, 1.0, 0.0) + (1.0 - factor) * vec3(0.0, 0.0, 0.0);
    
    outColor = vec4(color, 1.0);
}
