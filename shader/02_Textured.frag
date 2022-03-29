#version 460

// Light & view dir calculated in vertex shader for normal maps
// In addition we will use these in phong model so we can optimize our model further
in vec3 LightDir;
in vec3 ViewDir;
in vec2 TexCoord;

layout(binding=0) uniform sampler2D ColorTex;
layout(binding=1) uniform sampler2D NormalMapTex;
layout(binding=2) uniform sampler2D AdditionalColorTex;

layout(location=0) out vec4 FragColor;

struct LightInfo
{
	vec4 Position;  // Not used, this is calculated in vert shader. Leaving this for possible future usage.
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

// User-specified properties
uniform bool UseBlinnPhong = true;
uniform bool UseTextures = true;
uniform bool UseTextureMix = true;

uniform bool UseToon = false;
uniform bool UseAdditiveToon = false;
uniform float ToonFraction = 1.0f;

void phongModelHalfVector( vec3 normal, out vec3 outColor, out vec3 outSpec, out float specIntensity )
{
    // (H) calculate the half vector between the light vector and the view vector
	vec3 halfDir = normalize( ViewDir + LightDir );

    // With ambient light intensity
	vec3 ambient = Light.La * Material.Ka;

    // Is the pixel lit?
	specIntensity = max( dot( LightDir, normal ), 0.0 ); // I wish OpenGL had saturate(); function

    // With diffuse light intensity
	vec3 diffuse = Light.Ld * specIntensity;

    // If the vertex is lit, compute the specular color
	// Note that we create dot product of halfDir & normals
	if( specIntensity > 0.0 )
		outSpec = Light.Ls * Material.Ks * pow( max( dot( halfDir , normal ), 0.0 ), Material.Shininess );

    outColor = ambient + diffuse;
}

void phongModel( vec3 normal, out vec3 outColor, out vec3 outSpec, out float specIntensity )
{
    // (R) Reflection direction
    vec3 reflectDir = reflect( -LightDir, normal );

    // With ambient light intensity
    vec3 ambient = Light.La * Material.Ka;

    // Is the pixel lit?
    specIntensity = max( dot(LightDir, normal), 0.0 );

    // With diffuse light intensity
    vec3 diffuse = Light.Ld * specIntensity;

    // If the vertex is lit, compute the specular color
    if( specIntensity > 0.0 )
        outSpec = Light.Ls * Material.Ks * pow( max( dot(reflectDir, ViewDir), 0.0 ), Material.Shininess );

    outColor = ambient + diffuse;
}

void main()
{
    // Lookup the normal from the normal map
    vec4 normal = 2.0 * texture( NormalMapTex, TexCoord ) - 1.0;

    // Extract pixel color in tex0 and tex2
    vec4 tex0Color = texture( ColorTex, TexCoord );
    vec4 tex2Color = texture( AdditionalColorTex, TexCoord );

    vec3 outColor, outSpec;
    float outSpecIntensity;

    // Pick phong model
    if(UseBlinnPhong)
        phongModelHalfVector(normal.xyz, outColor, outSpec, outSpecIntensity);
    else
        phongModel(normal.xyz, outColor, outSpec, outSpecIntensity);

    // Pick texture method
    if(UseTextures)
    {
        if(UseTextureMix)
        {
            vec4 mixedTexColor = mix(tex0Color, tex2Color, tex2Color.a);
            FragColor = vec4(outColor, 1.0 ) * mixedTexColor + vec4( outSpec, 1 );
        }
        else
        {
            FragColor = vec4(outColor, 1.0 ) * tex0Color + vec4( outSpec, 1 );
        }
    }
    else
    {
        FragColor = vec4(outColor, 1.0 ) + vec4( outSpec, 1 );
    }

    // Apply toon model if enabled
    if (UseToon)
    {
        vec4 outToonColor;
        
        if (outSpecIntensity > pow(0.95, ToonFraction))
          outToonColor = vec4(vec3(1.0), 1.0);
        else if (outSpecIntensity > pow(0.5, ToonFraction))
          outToonColor = vec4(vec3(0.6), 1.0);
        else if (outSpecIntensity > pow(0.25, ToonFraction))
          outToonColor = vec4(vec3(0.4), 1.0);
        else
          outToonColor = vec4(vec3(0.2), 1.0);

        // Toonify current phong reflection
        if (UseAdditiveToon)
        {
            FragColor = outToonColor * FragColor; // This frag is already mixed above with phong model
        }
        // Otherwise, use pure toon effect (much more sharper)
        else
        {
            // Mixing toon color with tex0 & tex2
            if(UseTextureMix)
            {
                vec4 mixedTexColor = mix(tex0Color, tex2Color, tex2Color.a);
                FragColor = outToonColor * mixedTexColor;
            }
            else
            {
                // Just use base texture diffuse
                FragColor = outToonColor * tex0Color;
            }
        }
    }
}
