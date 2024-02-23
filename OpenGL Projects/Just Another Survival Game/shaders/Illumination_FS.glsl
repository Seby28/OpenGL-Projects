#version 330

// Input
in vec3 vWorldPosition;
in vec3 vWorldNormal;

// Uniform properties
uniform vec3 uPointLightPosition1;
uniform vec3 uPointLightPosition2;
uniform vec3 uSpotLightPositions[52];
uniform vec3 uSpotLightDirections[52];

uniform vec3 uEyePosition;

uniform vec3 uColor;

// Output
layout(location = 0) out vec4 outColor;

float ComputeDistanceAttenuation(vec3 lightPosition, vec3 pointPosition)
{
    float d = length(lightPosition - pointPosition);
    float attenuationFactor = 1.0 / (d * d + 1.0);

    return attenuationFactor;
}

vec3 ComputePhongIllumination(vec3 lightPosition)
{
    vec3 N = normalize(vWorldNormal);
    vec3 L = normalize(lightPosition - vWorldPosition);
    vec3 diffuseComponent = uColor * max(dot(N, L), 0.0);

    vec3 V = normalize(uEyePosition - vWorldPosition);
    vec3 R = reflect(-L, N);
    vec3 specularComponent = uColor * pow(max(dot(V, R), 0.0), 30);

    if (length(diffuseComponent) < 0.0)
    {
        specularComponent = vec3(0.0);
    }

    vec3 illumination = diffuseComponent + specularComponent;

    return illumination;
}

vec3 ComputePointLights()
{
    vec3 illumination = vec3(0.0);

    {
        vec3 lightPosition = uPointLightPosition1;
        vec3 lightColor = vec3(1);
    
        vec3 lightIllumination  = ComputePhongIllumination(lightPosition);
        float attenuationFactor = ComputeDistanceAttenuation(lightPosition, vWorldPosition);
    
        illumination += lightIllumination * lightColor * attenuationFactor;
    }

    {
        vec3 lightPosition = uPointLightPosition2;
        vec3 lightColor = vec3(1);
    
        vec3 lightIllumination  = ComputePhongIllumination(lightPosition);
        float attenuationFactor = ComputeDistanceAttenuation(lightPosition, vWorldPosition);
    
        illumination += lightIllumination * lightColor * attenuationFactor;
    }

    return illumination;
}

vec3 ComputeSpotLights()
{
    vec3 illumination = vec3(0.0);

    for (int i = 0; i < 52; i++)
    {
        vec3 lightPosition = uSpotLightPositions[i];
        vec3 lightColor = vec3(1.0);
        vec3 lightDirection = uSpotLightDirections[i];
        float cutoffAngle = 60.0;

        vec3 L = normalize(lightPosition - vWorldPosition);
        float cosTheta = dot(-L, lightDirection);
        float cosPhi = cos(cutoffAngle);
        if (cosTheta > cosPhi)
        {
            vec3 lightIllumination  = ComputePhongIllumination(lightPosition);
            float attenuationFactor = ComputeDistanceAttenuation(lightPosition, vWorldPosition);

            float linearAttenuationFactor = (cosTheta - cosPhi) / (1.0 - cosPhi);
            float quadraticAttenuationFactor = pow(linearAttenuationFactor, 2);

            illumination += lightIllumination * lightColor * attenuationFactor * quadraticAttenuationFactor * 5.0;
        }
    }

    return illumination;
}

vec3 ComputeAmbientLight()
{
    vec3 globalAmbientColor = vec3(0.1);

    vec3 ambientComponent = uColor * globalAmbientColor;

    return ambientComponent;
}

void main()
{
    vec3 illumination = ComputeAmbientLight() + ComputePointLights()
                      + ComputeSpotLights();

    outColor = vec4(illumination, 1);
}
