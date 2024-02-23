#version 330

// Input
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTextureCoord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Output value to fragment shader
out vec2 vTextureCoord;

void main()
{
    vTextureCoord = aTextureCoord;

    gl_Position = Projection * View * Model * vec4(aPosition, 1.0);
}
