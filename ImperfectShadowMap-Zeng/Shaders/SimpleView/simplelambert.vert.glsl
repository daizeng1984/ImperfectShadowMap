#version 400 
#pragma IGLU_ENABLE  DEPTH_TEST

layout(location = IGLU_VERTEX) in vec3 vertex;   // Sends vertex data from models here
layout(location = IGLU_NORMAL) in vec3 normal;   // Sends normal data (if any) from models here
layout(location = IGLU_MATL_ID)  in float matlID;  // Sends ID for current material from model (if any)

// The buffer containing our material information
uniform samplerBuffer  matlInfoTex;

uniform mat4 model;	 // Transforms: model space -> world space
uniform mat4 view;   // Transforms: world space -> eye space
uniform mat4 proj;   // Transforms: eye space   -> clip space

out vec4 esFragPos;
out vec4 esFragNormal;
out vec3 fragDiffuse;

void main( void )
{  
	// Store the eye-space position of this vertex, send to frag shader
	esFragPos = model * vec4( vertex, 1.0f );

	// Transform vertex normal to eye-space, send to frag shader
	esFragNormal = inverse( transpose( model ) ) * vec4( normal, 0.0 );	

	// look up the material and output the diffuse color
	int matlIdx     = 4*int(matlID) + 1;
	fragDiffuse   = texelFetch( matlInfoTex, matlIdx ).xyz;  // Get diffuse color
	

    // Transform vertex into clip space
	gl_Position = proj * ( view * model * vec4( vertex, 1.0f ) );	
}