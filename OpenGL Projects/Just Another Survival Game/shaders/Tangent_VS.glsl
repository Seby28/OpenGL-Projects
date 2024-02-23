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
out vec3 vWorldPosition;
out vec3 vWorldNormal;
out vec2 vTextureCoord;

out vec3 fragTangent;
out vec3 fragBitangent;

void main()
{
    vWorldPosition = (Model * vec4(aPosition, 1.0)).xyz;
    vWorldNormal = normalize(aNormal);
    // vWorldNormal = aNormal;

    vTextureCoord = aTextureCoord;

    // vec3 dp1 = dFdx(aPosition);
    // vec3 dp2 = dFdy(aPosition);


    // Pass position and texture coordinates to the fragment shader

    // Calculate tangent and bitangent in the vertex shader
    vec3 c1 = cross(aNormal, vec3(0.0, 0.0, 1.0));
    vec3 c2 = cross(aNormal, vec3(0.0, 1.0, 0.0));

    // Ensure the cross product vectors are not zero
    fragTangent = normalize(length(c1) > length(c2) ? c1 : c2);
    fragBitangent = normalize(cross(vWorldNormal, fragTangent));

    // Pass transformed tangent and bitangent to the fragment shader
    // Ensure that these are normalized in case they were modified during calculations
    fragTangent = normalize(fragTangent);
    fragBitangent = normalize(fragBitangent);

    gl_Position = Projection * View * Model * vec4(aPosition, 1.0);
}
