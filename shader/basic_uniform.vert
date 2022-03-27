#version 460

//in variables, this are in model coordinates
layout (location = 0) in vec3 VertexPosition; 
layout (location = 1) in vec3 VertexNormal; 

//out vector needed for the fragment shader
out vec3 Color; 

//light information struct
uniform struct LightInfo {
	vec4 Position; // Light position in eye coords.
	vec3 La; // Ambient light intensity
	vec3 L; // Diffuse and specular light intensity
} lights[3];

uniform struct MaterialInfo {
	vec3 Ka; // Ambient reflectivity
	vec3 Kd; // Diffuse reflectivity
	vec3 Ks; // Specular reflectivity
	float Shininess; // Specular shininess factor
} Material;

vec3 phongModel(int light, vec3 position, vec3 n)
{
	LightInfo currentLight = lights[light];

	// Ambient calculation
	vec3 ambient = Material.Ka * currentLight.La;

	// Diffuse calculation
	vec3 s = Material.Kd * currentLight.L;

	float sDotN = max(dot(s,n), 0.0);
	vec3 diffuse = Material.Kd * sDotN;

	// Specular calculation
	vec3 spec = vec3(0.0);

	if (sDotN > 0.0)
	{
		vec3 v = normalize(-position.xyz);
		vec3 r = reflect( -s, n );
		spec = Material.Ks * pow( max( dot(r,v), 0.0 ), Material.Shininess );
	}

	return ambient + currentLight.L * (diffuse + spec);
}

//uniforms for matrices required in the shader
uniform mat4 ModelViewMatrix;   //model view matrix
uniform mat3 NormalMatrix;		//normal matrix
uniform mat4 MVP;				//model view projection matrix

void main() 
{
	// convert the vertex normal to eye coordinates
	vec3 n = normalize( NormalMatrix * VertexNormal);

	// convert the vertex position to eye coordinates
	vec4 pos = ModelViewMatrix * vec4(VertexPosition,1.0);

	// evaluate the lighting equation for each light
	Color = vec3(0.0);
	vec3 camCoords = (ModelViewMatrix * vec4(VertexPosition, 1.0)).xyz;

	for(int i = 0; i < 3; i++)
		Color += phongModel(i, camCoords, n);

	//turns any vertex position into model view projection in preparations to 
	//graphics pipeline processes before fragment shader (clipping)
	gl_Position = MVP * vec4(VertexPosition,1.0); 
}
