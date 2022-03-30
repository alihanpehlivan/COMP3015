#version 460

//
// \brief Vertex shader for diffuse single texture fragment shader.
// Passes TexCoord to fragment shader.
// Uses DiffuseSingleTexture fragment shader.
// 

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal; // Not used
layout (location = 2) in vec2 VertexTexCoord;

layout (location = 0) out vec2 TexCoord;

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

void main()
{
	TexCoord = VertexTexCoord;
	gl_Position = MVP * vec4(VertexPosition,1.0);
}
