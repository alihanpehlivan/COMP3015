#version 460

in vec3 LightDir;
in vec3 ViewDir;
in vec2 TexCoord;

layout(binding=0) uniform sampler2D ColorTex;
layout(binding=1) uniform sampler2D NormalMapTex;

layout(location=0) out vec4 FragColor;

struct LightInfo
{
	vec4 Position;  // Light position in eye coords
	vec3 La;        // Ambient light intensity
	vec3 Ld;        // Diffuse light intensity
	vec3 Ls;        // Specular light intensity
};

uniform LightInfo Light;

struct MaterialInfo
{
  vec3 Ka;          // Ambient reflectivity
  vec3 Ks;          // Specular reflectivity
  float Shininess;  // Specular shininess factor
};

uniform MaterialInfo Material;

// Settings
uniform bool UseBlinnPhong = true;

// Blinn-Phong Model
vec3 phongModelHalfVector(vec3 normal, vec3 texColor)
{
    // (H) calculate the half vector between the light vector and the view vector
	vec3 halfDir = normalize( ViewDir + LightDir );

    // With ambient light intensity
	vec3 ambient = Light.La * Material.Ka;

    // Is the pixel lit?
	float specAngle = max( dot( LightDir, normal ), 0.0 ); // I wish OpenGL had saturate(); function

    // With diffuse light intensity
	vec3 diffuse = Light.Ld * specAngle;

	vec3 spec = vec3(0.0);

    // If the vertex is lit, compute the specular color
	// Note that we create dot product of halfDir & normals
	if( specAngle > 0.0 )
		spec = Light.Ls * Material.Ks * pow( max( dot( halfDir , normal ), 0.0 ), Material.Shininess );

    // Final pixel color with tex color
    return (ambient + diffuse) * texColor + spec;
}

vec3 phongModel( vec3 normal, vec3 texColor )
{
    // (R) Reflection direction
    vec3 reflectDir = reflect( -LightDir, normal );

    // With ambient light intensity
    vec3 ambient = Light.La * Material.Ka;

    // Is the pixel lit?
    float specAngle = max( dot(LightDir, normal), 0.0 );

    // With diffuse light intensity
    vec3 diffuse = Light.Ld * specAngle;

    vec3 spec = vec3(0.0);

    // If the vertex is lit, compute the specular color
    if( specAngle > 0.0 )
        spec = Light.Ls * Material.Ks * pow( max( dot(reflectDir, ViewDir), 0.0 ), Material.Shininess );

    return (ambient + diffuse) * texColor + spec;
}

void main()
{
    // Lookup the normal from the normal map
    vec4 normal = 2.0 * texture( NormalMapTex, TexCoord ) - 1.0;

    // Extract pixel color in the texture coord
    vec4 texColor = texture( ColorTex, TexCoord );
    
    if(UseBlinnPhong)
        FragColor = vec4( phongModelHalfVector(normal.xyz, texColor.rgb), 1.0 );
    else
        FragColor = vec4( phongModel(normal.xyz, texColor.rgb), 1.0 );
}
