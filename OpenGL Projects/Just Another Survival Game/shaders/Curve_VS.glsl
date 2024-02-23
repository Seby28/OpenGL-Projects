#version 330

// Input
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTextureCoord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

uniform vec3 uPlayerPosition;
uniform float uCurvatureFactor;

// Output value to fragment shader
out vec3 vWorldPosition;
out vec3 vWorldNormal;

void main()
{
    vec3 vertexPosition = vec3(Model * vec4(aPosition, 1));
    vertexPosition.y = vertexPosition.y - pow(length(uPlayerPosition - vertexPosition), 2) * uCurvatureFactor;
    
    vWorldPosition = (Model * vec4(aPosition, 1.0)).xyz;
    vWorldNormal = mat3(transpose(inverse(Model))) * aNormal;

    gl_Position = Projection * View * vec4(vertexPosition, 1.0);
}
