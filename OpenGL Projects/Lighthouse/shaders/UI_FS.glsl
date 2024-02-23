#version 330

// Uniform properties
uniform vec3 color;

// Output
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(color, 1.0);
}
