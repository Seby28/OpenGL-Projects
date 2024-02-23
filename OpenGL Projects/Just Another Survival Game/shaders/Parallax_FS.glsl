#version 430

// Input
layout(location = 0) in vec2 vTextureCoord;
layout(location = 1) in vec3 vWorldPosition;
layout(location = 2) in vec3 vWorldNormal;
layout(location = 3) in vec3 fragTangent;
layout(location = 4) in vec3 fragBitangent;

// Uniform properties
uniform sampler2D uTexture;
uniform ivec2 uResolution;
uniform float uTime;
uniform bool uRave;

uniform vec3 uEyePosition;

// Output
layout(location = 0) out vec4 outColor;

mat2 rotate2d(float angle)
{
    return mat2(cos(angle), -sin(angle),
                sin(angle),  cos(angle));
}

float grid(vec2 uv, float width)
{
    return step(vec2(width), uv).x * step(vec2(width), uv).y;
}

void main()
{


    vec2 uv = vTextureCoord;
    vec3 color = vec3(0.0);


    // uv = rotate2d((uTime)) * uv;


    vec3 tangent = abs(vWorldNormal) / vec3(uv.x);
    
    vec3 angle = atan(vWorldNormal, vec3(uv.x));

     // uv -= vec2(0.5);
    /// / rotate the space
     // uv = rotate2d(uTime) * uv;
    // move it back to the original place
    //uv += vec2(0.5);

     vec3 p_dx = dFdx(vWorldPosition);
     vec3 p_dy = dFdy(vWorldPosition);
     // compute derivations of the texture coordinate
     vec2 tc_dx = dFdx(uv);
     vec2 tc_dy = dFdy(uv);
     // compute initial tangent and bi-tangent
     vec3 t = normalize( tc_dy.y * p_dx - tc_dx.y * p_dy );
     vec3 b = normalize( tc_dy.x * p_dx - tc_dx.x * p_dy ); // sign inversion
     // get new tangent from a given mesh normal
     vec3 n = normalize(vWorldNormal);
     vec3 x = cross(n, t);
     t = cross(x, n);
     t = normalize(t);
     // get updated bi-tangent
     x = cross(b, n);
     b = cross(n, x);
     b = normalize(b);
     mat3 tbn = mat3(t, b, n);

    vec3 V = normalize(uEyePosition - vWorldPosition);

    float a1 = dot(V, t);
    float a2 = dot(V, b);

    vec3 combineDot = vec3(a1, a2, 0) * 2.0;

    vec3 normal = tbn * vWorldNormal;

    vec2 parallax = uv - combineDot.xy;


    vec3 texColor = texture(uTexture, parallax.xy).xyz;

    vec2 st = combineDot.xy;
    
    // st.x *= 8.0;
    // st.y *= 2.0;
    // st = fract(st);
    st *= 1.0;
    // st = fract(st);
    st += vec2(0.5);

    // Bottom-left
    float cell = grid(st, 0.025);

    // Top-right
    cell *= grid(1.0 - st, 0.025);

    vec3 grid = vec3(cell) * vec3(0.0) + (1.0 - vec3(cell)) * vec3(0.0, 1.0, 0.0);


    outColor = vec4((uRave) ? combineDot : 1.0 - combineDot, 1.0);


    // if (outColor.xyz == vec3(0.0))
    //     discard;
}
