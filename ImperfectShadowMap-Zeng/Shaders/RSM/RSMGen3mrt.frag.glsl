//Output All RSM ( flux, normal, world position)
#version 400
#extension GL_EXT_gpu_shader4 : enable
//Enable the depth test
#pragma IGLU_DISABLE BLENDING

// Output buffers
layout(location = 0) out vec4 fragOutput0;
layout(location = 1) out vec4 fragOutput1;
layout(location = 2) out vec4 fragOutput2;

// Common shader funcition for parabolic shadow map projection
#include ../Shaders/Common/common.h.glsl

uniform mat4 view;   // Transforms: world space -> eye space

in vec4 esFragPos; 
in vec4 esFragNormal;
in vec3 fragDiffuse;

uniform float lightIntensity = 1.0;
uniform float maxConeAngle = 50; // In degree! and notice it's half angle used in perspective computation


void main( void ) {
	//Output flux, now only dependent on normal, we assume all pixel light beam output the same radiance.
	vec3 EtoF = normalize(0 -(view *esFragPos).xyz);
	vec3 normal = TrasformNormal(view, esFragNormal.xyz);
	//Check the spot cone
	vec3 LDir = vec3(0,0,-1);
	float inLightCone = dot(-EtoF, LDir) > cos(M_Pi*maxConeAngle/180.0) ? 1.0 : 0.0;

	fragOutput0 = inLightCone*lightIntensity*max(0, dot(EtoF, normal))*vec4(fragDiffuse,1);

	//Output world coordinates
	fragOutput1 = inLightCone*esFragPos;

	//Output normal
	fragOutput2 = inLightCone*vec4(normalize(esFragNormal.xyz), 1);

}