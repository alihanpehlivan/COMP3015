#version 460

// Per Fragment Blinn and BlinnPhong reflection

layout (location = 0) in vec3 Position; // Interpolated position for this fragment
layout (location = 1) in vec3 Normal; // Interpolated normal for this fragment

// For optimized Blinn and Phong model
layout (location = 2) in vec3 LightDir; // (L) 3D position in space of the surface
layout (location = 3) in vec3 HalfDir; // (H) Blinn-Phong: calculate the half vector between the light vector and the view vector

out vec3 Color;

//light information struct
uniform struct LightInfo {
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

uniform bool UseBlinnPhong = true;

// Normal Phong Model
vec3 phongModel(vec3 position, vec3 normal)
{
	// Continually calculate the dot product R.V between a viewer (V)
	// and the beam from a light-source (L) reflected (R) on a surface.

	// (N) normal is already normalized by vertex shader

	// (V) View direction
	vec3 viewDir = normalize(-position);
	
	// (R) Reflection direction
	vec3 reflectDir = reflect( -LightDir, normal );

	// 1. Calculate ambient color
	vec3 ambient = Light.La * Material.Ka;

	// Is the vertex lit?
	float specAngle = max( dot( LightDir, normal ), 0.0 ); // I wish OpenGL had saturate(); function
	
	// 2. Calculate diffuse color
	vec3 diffuse = Light.Ld * Material.Kd * specAngle;

	// Set the default specular term to black
	vec3 spec = vec3(0.0);

	// 3. If the vertex is lit, compute the specular color
	if( specAngle > 0.0 )
	{
		spec = Light.Ls * Material.Ks * pow( max( dot( reflectDir, viewDir ), 0.0 ), Material.Shininess );
	}

	return ambient + diffuse + spec;
}

// Non optimized Blinn-Phong Model
vec3 phongModelHalfVector(vec3 position, vec3 normal)
{
	// (N) normal is already normalized by vertex shader

	// (V) View direction
	vec3 viewDir = normalize(-position);

	// (H) Blinn-Phong: calculate the half vector between the light vector and the view vector
	vec3 halfDir = normalize( viewDir + LightDir );

	// 1. Calculate ambient color
	vec3 ambient = Light.La * Material.Ka;

	// Is the vertex lit?
	float specAngle = max( dot( LightDir, normal ), 0.0 ); // I wish OpenGL had saturate(); function

	// 2. Calculate diffuse color
	vec3 diffuse = Light.Ld * Material.Kd * specAngle;

	// 3. If the vertex is lit, compute the specular color
	vec3 spec = vec3(0.0);
	if( specAngle > 0.0 )
	{
		// Note that we create dot product of halfDir & normals
		spec = Light.Ls * Material.Ks * pow( max( dot( halfDir , normal ), 0.0 ), Material.Shininess );
	}

	return ambient + diffuse + spec;
}

vec3 optimizedPhongModelHalfVector(vec3 normal)
{
	// (N) normal is already normalized by vertex shader

	// 1. Calculate ambient color
	vec3 ambient = Light.La * Material.Ka;

	// Is the vertex lit?
	float specAngle = max( dot( LightDir, normal ), 0.0 ); // I wish OpenGL had saturate(); function

	// 2. Calculate diffuse color
	vec3 diffuse = Light.Ld * Material.Kd * specAngle;

	// 3. If the vertex is lit, compute the specular color
	vec3 spec = vec3(0.0);
	if( specAngle > 0.0 )
	{
		// Note that we create dot product of halfDir & normals
		spec = Light.Ls * Material.Ks * pow( max( dot( HalfDir , normal ), 0.0 ), Material.Shininess );
	}

	return ambient + diffuse + spec;
}

void main()
{
	if(UseBlinnPhong)
	{
		// Optimized Blinn-Phong:
		// I took the tip from: https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_reflection_model#Efficiency
		// The H vector is independent of position and surface curvature simply because
		// the H vector is dependent on the direction to V position and the
		// direction to the L, which individually converge at this remote distance,
		// hence the H vector can be thought of as constant in this case.

		//Color = phongModelHalfVector(Position, Normal);
		Color = optimizedPhongModelHalfVector(Normal);
	}
	else
	{
		Color = phongModel(Position, Normal);
	}
}
