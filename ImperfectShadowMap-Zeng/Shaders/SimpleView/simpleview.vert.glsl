#version 400 
#pragma IGLU_ENABLE  DEPTH_TEST

layout(location = IGLU_VERTEX) in vec3 vertex;   // Sends vertex data from models here

uniform mat4 model;	 // Transforms: model space -> world space
uniform mat4 view;   // Transforms: world space -> eye space
uniform mat4 proj;   // Transforms: eye space   -> clip space

out vec4 esFragPos;


void main( void )
{  
	// Store the eye-space position of this vertex, send to frag shader
	esFragPos = model * vec4( vertex, 1.0f );

    // Transform vertex into clip space
	gl_Position = proj * ( view * model * vec4( vertex, 1.0f ) );	
}