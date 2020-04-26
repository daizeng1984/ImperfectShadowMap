#version 400 

layout(location = IGLU_VERTEX) in vec3 vertex;   // Sends vertex data from models here
layout(location = IGLU_NORMAL) in vec3 normal;   // Sends normal data (if any) from models here

uniform mat4 model;	 // Transforms: model space -> world space
uniform mat4 view;   // Transforms: world space -> eye space
uniform mat4 proj;   // Transforms: eye space   -> clip space

out vec4 fragPos;
out vec3 fragNormal;

void main( void )
{  
	// Store the raw position of this vertex, send to frag shader
	fragPos = vec4( vertex, 1.0f );
	
	// Transform vertex normal to eye-space, send to frag shader
	fragNormal = normal;	

    // Transform vertex into clip space
	gl_Position = proj * ( view * model * vec4( vertex, 1.0f ) );	
}