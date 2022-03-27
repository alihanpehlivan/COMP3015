#version 460

// Color computed per fragment

layout (location = 0) in vec3 Position; // Interpolated position for this fragment
layout (location = 1) in vec3 Normal; // Interpolated normal for this fragment

out vec3 Color;

//light information struct
uniform struct LightInfo {
	vec4 Position; // Light position in eye coords
	vec3 La; // Ambient light intensity
	vec3 Ld; // Diffuse light intensity
	vec3 Ls; // Specular light intensity
} Light;

uniform struct MaterialInfo {
	vec3 Ka; // Ambient reflectivity
	vec3 Kd; // Diffuse reflectivity
	vec3 Ks; // Specular reflectivity
	float Shininess; // Specular shininess factor
} Material;

vec3 phongModel(vec3 position, vec3 n)
{
	vec3 s = normalize(Light.Position.xyz - position);
	vec3 v = normalize(-position);
	vec3 r = reflect( -s, n );

	// 1. Calculate ambient color
	vec3 ambient = Light.La * Material.Ka;

	// Is the vertex lit?
	float sDotN = max( dot(s, n), 0.0 );
	
	// 2. Calculate diffuse color
	vec3 diffuse = Light.Ld * Material.Kd * sDotN;

	// Set the default specular term to black
	vec3 spec = vec3(0.0);

	// 3. If the vertex is lit, compute the specular color
	if( sDotN > 0.0 )
	{
		spec = Light.Ls * Material.Ks * pow( max( dot(r,v), 0.0 ), Material.Shininess );
	}

	return ambient + diffuse + spec;
}

void main() 
{
	Color = phongModel(Position, Normal);
}
