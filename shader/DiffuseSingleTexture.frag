#version 460

//
// \brief Simply extracts and uses color information from Tex0.
// Uses DiffuseSingleTexture vertex shader.
//

layout (binding = 0) uniform sampler2D Tex0;
layout (location = 0) in vec2 TexCoord;
layout (location = 0) out vec4 FragColor;

void main()
{
	FragColor = texture(Tex0, TexCoord);
}
