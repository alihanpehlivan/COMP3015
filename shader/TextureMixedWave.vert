#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;
layout (location = 3) in vec4 VertexTangent;

layout (location = 0) out vec2 TexCoord;
layout (location = 1) out vec3 LightDir;
layout (location = 2) out vec3 ViewDir;

// Uniform buffers
layout (std140, binding = 0) uniform Matrices
{
    mat4 MVP;
    mat4 ModelViewMatrix;
    mat3 NormalMatrix;
};

// Needed for seperate shaders
out gl_PerVertex
{
    vec4 gl_Position;
};

uniform vec4 LightPosition;
uniform float Time;
uniform float WaveFreq = 2.5;
uniform float WaveVelocity = 2.5;
uniform float WaveAmp = 0.6;

void main()
{
    vec4 pos = vec4(VertexPosition,1.0);

    float u = WaveFreq * pos.x - WaveVelocity * Time;
    pos.y = WaveAmp * sin( u );

    // new VertexNormal
    vec3 n = vec3(0.0);
    n.xy = normalize(vec2(cos( u ), 1.0));

    // Transform normal and tangent to eye space
    vec3 norm = normalize( NormalMatrix * n );
    vec3 tang = normalize( NormalMatrix * vec3(VertexTangent) );

    // Compute the binormal
    vec3 binormal = normalize( cross( norm, tang ) ) * VertexTangent.w;

    // Matrix for transformation to tangent space
    mat3 toObjectLocal = mat3(
        tang.x, binormal.x, norm.x,
        tang.y, binormal.y, norm.y,
        tang.z, binormal.z, norm.z);

    // Transform light direction and view direction to tangent space
    vec3 pos2 = vec3( ModelViewMatrix * pos );

    LightDir = normalize( toObjectLocal * (LightPosition.xyz - pos2) );
    ViewDir = toObjectLocal * normalize(-pos2);
    TexCoord = VertexTexCoord;

    gl_Position = MVP * pos;
}
