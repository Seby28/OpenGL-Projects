#version 330

// Uniform properties
uniform vec3 uColor;

// Output
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(uColor, 0.5);
}
