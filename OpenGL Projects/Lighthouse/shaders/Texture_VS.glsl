#version 330

// Input
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTextureCoord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Output value to fragment shader
out vec3 worldPosition;
out vec3 worldNormal;
out vec2 textureCoord;

void main()
{
    worldPosition = (Model * vec4(inPosition, 1.0)).xyz;
    worldNormal = mat3(transpose(inverse(Model))) * inNormal;

    textureCoord = inTextureCoord;

    gl_Position = Projection * View * Model * vec4(inPosition, 1.0);
}
