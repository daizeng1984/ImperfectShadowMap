// Draw multiple instance vpl based on the vplTexBuffer's pos, normal and flux
#version 400 
#pragma IGLU_ENABLE  DEPTH_TEST

layout(location = IGLU_VERTEX) in vec3 vertex;   // Sends vertex data from models here

// Common shader funcition for parabolic shadow map projection
#include ../Shaders/Common/common.h.glsl

// The vpl position buffer and normal buffer, it's textures that contains
//    vpl position and normal in RGB32F.
uniform samplerBuffer vplBuffer;

uniform mat4 view;   // Transforms: world space -> eye space
uniform mat4 proj;   // Transforms: eye space   -> clip space

uniform float scale = 0.1;
uniform float blendWeight = 0.6;
out vec4 color;


void main( void )
{
	//VPL's information pos, normal and flux for each instance
	vec4 vplPos = texelFetch( vplBuffer, 3*gl_InstanceID + 0 );
	vec4 vplNorm = texelFetch( vplBuffer, 3*gl_InstanceID + 1 );  //notice normal is different when tranforming
	vec4 vplFlux = texelFetch( vplBuffer, 3*gl_InstanceID + 2 ); 
	
	vec3 worldPos;
	// if we draw arrow to represent the light, Rotate to point as normal
#if 1
	vec3 binormal, tangent;
	Onb(vplNorm.xyz, binormal, tangent );
	worldPos = Onb2World( scale*vertex, -vplNorm.xyz, binormal, tangent); //TODO: ??? why minus 1
	worldPos = worldPos + vplPos.xyz;
#else
	worldPos = scale*vertex + vplPos.xyz;
#endif

    // Transform vertex into clip space
	gl_Position = proj*view*vec4(worldPos, 1);

	// compute the color based on normal
	//if(gl_InstanceID != 15)
		//color = vplFlux;
	//else
		//color = vec4(1,0,0,1);
	color = vplFlux*blendWeight;
}