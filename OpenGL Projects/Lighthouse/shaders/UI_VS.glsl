#version 330

// Input
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTextureCoord;

out vec2 textureCoord;

void main()
{
    textureCoord = aTextureCoord;

    gl_Position = vec4(aPosition, 1.0);
}
