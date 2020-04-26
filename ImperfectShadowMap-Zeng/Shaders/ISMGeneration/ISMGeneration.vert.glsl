#version 400 

layout(location = IGLU_VERTEX) in vec3 vertex;   // Sends vertex data from models here

uniform mat4 model;	 // Transforms: model space -> world space

out vec4 verColor; 

// Common shader funcition for parabolic shadow map projection
#include ../Shaders/Common/common.h.glsl

// N_set is an important parameters that indicates how many VPLs we are
//    gonna use. For N_set = 8, it means we are gonna use 8*8 vpls
uniform int N_set;
// The vpl position buffer and normal buffer, it's textures that contains
//    vpl position and normal in RGB32F.
uniform samplerBuffer vplBuffer;
// Scale the depth, so it's more clear when we show it
uniform bool scaleDepth = false;
// The near & far planes for projecting the vertex's z-distance, they are >0, so its range is (-n, -f)
uniform float nearDist, farDist;

void main( void )
{  
	// Store the eye-space position of this vertex, send to frag shader
	vec4 pos = model * vec4( vertex, 1.0f );

	//vec4 paraCoord = ComputeParabolicCoordinate(pos.xyz, eyePos , at - eyePos, nearDist, farDist);
	vec4 vplPos = texelFetch( vplBuffer, 3*gl_InstanceID + 0 );
	vec4 vplNorm = texelFetch( vplBuffer, 3*gl_InstanceID + 1 ); //notice normal is different when tranforming

	vec4 paraCoord = ComputeParabolicCoordinate(pos.xyz, vplPos.xyz,  vplNorm.xyz , nearDist, farDist);
	// Pass the negpos value as W to geom shader to discard vertex, since geometry shader is before rasterization
	gl_Position = paraCoord;

	//Compute whether the position is in pos or neg, we ignore neg in geom
	// set back to gl_position
	gl_Position.xy = GridNDC(gl_Position.xy, N_set, gl_InstanceID);
	
	// Test code to scale the depth
	if(scaleDepth)
	{
		gl_Position.z = pow(gl_Position.z, 32);
	}
	if(paraCoord.w < 0)
	{
		verColor = vec4(1,0,0,1);
	}
	else
	{
		verColor = vec4(0,1,0,1);
	}
}