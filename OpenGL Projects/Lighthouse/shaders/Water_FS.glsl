#version 330

// Input
in vec3 worldPosition;
in vec3 worldNormal;
in vec2 textureCoord;

// Uniform properties
uniform vec3 pointLightPositions[5];
uniform vec3 pointLightColors[5];
uniform vec3 spotLightPositions[2];
uniform vec3 spotLightDirections[2];

uniform vec3 lightDirection;
uniform vec3 eyePosition;

uniform sampler2D texture1;
uniform vec3 mainColor;

uniform float time;

// Output
layout(location = 0) out vec4 outColor;

vec3 ComputeDirectionalLight(vec3 material)
{
    vec3 N = normalize(worldNormal);
    vec3 L = normalize(-lightDirection);
    vec3 diffuseComponent = material * max(dot(N, L), 0.0);

    vec3 V = normalize(eyePosition - worldPosition);
    vec3 R = reflect(-L, N);
    vec3 specularComponent = material * pow(max(dot(V, R), 0.0), 30);

    if (length(diffuseComponent) < 0.0)
    {
        specularComponent = vec3(0.0);
    }

    vec3 illumination = (diffuseComponent + specularComponent) * vec3(0.8);

    return illumination;
}

float ComputeDistanceAttenuation(vec3 lightPosition, vec3 pointPosition)
{
    float d = length(lightPosition - pointPosition);
    float attenuationFactor = 1.0 / (d * d + 1.0);

    return attenuationFactor;
}

vec3 ComputePhongIllumination(vec3 lightPosition, vec3 material)
{
    vec3 N = normalize(worldNormal);
    vec3 L = normalize(lightPosition - worldPosition);
    vec3 diffuseComponent = material * max(dot(N, L), 0.0);

    vec3 V = normalize(eyePosition - worldPosition);
    vec3 R = reflect(-L, N);
    vec3 specularComponent = material * pow(max(dot(V, R), 0.0), 30);
    
    if (length(diffuseComponent) < 0.0)
    {
        specularComponent = vec3(0.0);
    }

    vec3 illumination = diffuseComponent + specularComponent;

    return illumination;
}

vec3 ComputePointLights(vec3 material)
{
    vec3 illumination = vec3(0.0);

    for (int i = 0; i < 4; i++)
    {
        vec3 lightPosition = pointLightPositions[i];
        vec3 lightColor = pointLightColors[i];

        vec3 lightIllumination  = ComputePhongIllumination(lightPosition, material);
        float attenuationFactor = ComputeDistanceAttenuation(lightPosition, worldPosition);

        illumination += lightIllumination * lightColor * attenuationFactor * 2.0;
    }

    {
        vec3 lightPosition = pointLightPositions[4];
        vec3 lightColor = mainColor;

        vec3 lightIllumination  = ComputePhongIllumination(lightPosition, material);
        float attenuationFactor = ComputeDistanceAttenuation(lightPosition, worldPosition);

        illumination += lightIllumination * lightColor * attenuationFactor * 2.0;
    }

    return illumination;
}

vec3 ComputeSpotLights(vec3 material)
{
    vec3 illumination = vec3(0.0);

    for (int i = 0; i < 2; i++)
    {
        vec3 lightPosition = spotLightPositions[i];
        vec3 lightColor = mainColor;
        vec3 lightDirection = spotLightDirections[i];
        float cutoffAngle = 120.0;

        vec3 L = normalize(lightPosition - worldPosition);
        float cosTheta = dot(-L, lightDirection);
        float cosPhi = cos(cutoffAngle);
        if (cosTheta > cosPhi)
        {
            vec3 lightIllumination = ComputePhongIllumination(lightPosition, material);
            float attenuationFactor = ComputeDistanceAttenuation(lightPosition, worldPosition);

            float linearAttenuationFactor = (cosTheta - cosPhi) / (1.0 - cosPhi);
            float quadraticAttenuationFactor = pow(linearAttenuationFactor, 2);

            illumination += lightIllumination * lightColor * quadraticAttenuationFactor * 2.0;
        }
    }

    return illumination;
}

vec3 ComputeAmbientLight(vec3 material)
{
    vec3 globalAmbientColor = vec3(0.1);

    vec3 ambientComponent = material * globalAmbientColor;

    return ambientComponent;
}

void main()
{
    vec2 uv = textureCoord;
    uv *= 16;

    float displacement = (0.2 * sin(worldPosition.x * 2.0 + time) + 1.0) * 0.5;
    vec2 modulatedCoords = uv + vec2(displacement, 0.0);

    vec2 translate = vec2(time * 0.05, 0.0);
    modulatedCoords += translate;

    vec3 color = texture(texture1, modulatedCoords).xyz;

    vec3 illumination = ComputeAmbientLight(color) 
                      + ComputeDirectionalLight(color) 
                      + ComputePointLights(color) 
                      + ComputeSpotLights(color);

    outColor = vec4(illumination, 1.0);
}
