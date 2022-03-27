#version 460

// Per Fragment Blinn and BlinnPhong reflection

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

out vec3 Position, Normal;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 MVP;

// For optimized Blinn Phong model
out vec3 LightDir, HalfDir;
uniform vec4 LightPosition;

void main()
{
	// Phong model optimization
	LightDir = normalize(LightPosition.xyz - Position); // (L) 3D position in space of the surface
	// Optimized Blinn Phong model
	HalfDir = normalize( normalize(-Position) + LightDir ); // (V) View direction + (H) calculate the half vector between the light vector and the view vector


	Normal = normalize( NormalMatrix * VertexNormal);
	Position = (ModelViewMatrix * vec4(VertexPosition,1.0)).xyz;
	gl_Position = MVP * vec4(VertexPosition,1.0);
}
