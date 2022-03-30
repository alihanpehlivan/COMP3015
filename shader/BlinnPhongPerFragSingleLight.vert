#version 460

//
// \brief ...
// 

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

layout (location = 0) out vec3 Position;
layout (location = 1) out vec3 Normal;
layout (location = 2) out vec3 LightDir;
layout (location = 3) out vec3 HalfDir;
layout (location = 4) out vec3 ViewDir;

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

void main()
{
	Normal = normalize( NormalMatrix * VertexNormal);
	Position = (ModelViewMatrix * vec4(VertexPosition, 1)).xyz;

	LightDir = normalize(LightPosition.xyz - Position); // (L) 3D position in space of the surface
	HalfDir = normalize( normalize(-Position) + LightDir ); // (V) View direction + (H) calculate the half vector between the light vector and the view vector
	ViewDir = normalize(-Position);

	gl_Position = MVP * vec4(VertexPosition, 1);
}
