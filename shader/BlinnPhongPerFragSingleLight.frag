#version 460

//
// \brief Blinn-Phong reflection model with single light.
// Uses BlinnPhongPerFragSingleLight vertex shader.
//

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec3 LightDir;
layout (location = 3) in vec3 HalfDir;
layout (location = 4) in vec3 ViewDir;

layout (location = 0) out vec4 FragColor;

struct LightInfo
{
	vec3 La; // Ambient light intensity
	vec3 Ld; // Diffuse light intensity
	vec3 Ls; // Specular light intensity
};

struct MaterialInfo
{
  vec3 Ka;          // Ambient reflectivity
  vec3 Ks;          // Specular reflectivity
  float Shininess;  // Specular shininess factor
};

uniform LightInfo Light;
uniform MaterialInfo Material;

vec3 phongModelHalfVector(vec3 normal)
{
    // (H) calculate the half vector between the light vector and the view vector
	vec3 halfDir = normalize(ViewDir + LightDir);

    // With ambient light intensity
	vec3 ambient = Light.La * Material.Ka;

    // Is the pixel lit?
	float specIntensity = max(dot(LightDir, normal), 0.0);

    // With diffuse light intensity
	vec3 diffuse = Light.Ld * specIntensity;

	vec3 spec = vec3(0.0);

    // If the vertex is lit, compute the specular color
	// Note that we create dot product of halfDir & normals
	if(specIntensity > 0.0)
		spec = Light.Ls * Material.Ks * pow(max(dot(halfDir, normal), 0), Material.Shininess);

    return ambient + diffuse + spec;
}

void main()
{
	FragColor = vec4(phongModelHalfVector(Normal.xyz), 1.0);
}
