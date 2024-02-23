#version 430

// Input
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTextureCoord;

// Output
layout(location = 0) out vec2 vTextureCoord;

void main()
{
    vTextureCoord = aTextureCoord;

    gl_Position = vec4(aPosition, 1.0);
}
