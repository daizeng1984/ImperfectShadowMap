#version 400 

layout(location = IGLU_VERTEX) in vec3 vertex;   // Sends vertex data from models here
layout(location = IGLU_MATERIAL) in int materialId;  //TODO:

uniform mat4 model;	 // Transforms: model space -> world space
uniform mat4 view;   // Transforms: world space -> eye space
uniform mat4 proj;   // Transforms: eye space   -> clip space

out float esFragColor; 
//out vec4 esFragPos;
uniform int N_set; // means N_set x N_set set

#include ../Shaders/Common/common.h.glsl

// Information about the near & far planes for projecting the vertex's z-distance
const float nearDist = 0.1, farDist = 5.0;
void main( void )
{  
	// Store the eye-space position of this vertex, send to frag shader
	//esFragPos = view * model * vec4( vertex, 1.0f );
	esFragColor = materialId==materialId? 1: 0.1;
	
	vec4 pos = model * vec4( vertex, 1.0f );

	gl_Position = proj * ( view * model * vec4( vertex, 1.0f ));	

	// set back to gl_position
	vec2 xy = GridNDC(gl_Position.xy/gl_Position.w, N_set, materialId);
	gl_Position.xy = xy*gl_Position.w;
}