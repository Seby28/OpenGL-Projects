#version 330

// Input
in vec2 vTextureCoord;

uniform float uTime;

uniform vec3 uHealthBarColor;
uniform float uFrequency;
uniform float uSpeed;

// Output
layout(location = 0) out vec4 outColor;

float grid(vec2 uv, float width)
{
    return step(vec2(width), uv).x * step(vec2(width), uv).y;
}

float plot(vec2 uv, float edge)
{
    return smoothstep(edge - 0.05, edge, uv.y) - smoothstep(edge, edge + 0.05, uv.y);
}

void main()
{
    vec2 st = vTextureCoord;
    
    st.x *= 8.0;
    st.y *= 2.0;
    st = fract(st);

    // Bottom-left
    float cell = grid(st, 0.025);

    // Top-right
    cell *= grid(1.0 - st, 0.025);

    vec3 grid = vec3(cell) * vec3(0.0) + (1.0 - vec3(cell)) * uHealthBarColor / 2.0;
    
    vec2 uv = vTextureCoord;
    uv.x *= 4;

    float y = (sin((uv.x + uTime * uSpeed) * uFrequency) + 1.0) / 2.0;
    vec3 color = vec3(y);

    float graph = plot(uv, 1.0 - y);
    color = (1.0 - graph) * color * grid + (1.0 - graph) * (1.0 - color) * grid + graph * uHealthBarColor;
    
    outColor = vec4(color, 1.0);
}
