// This is a shader simple view

#version 400
#pragma IGLU_DISABLE  BLENDING

out vec4 result;

//Input from fragment shader
in vec4 esFragPos;
in vec4 esFragNormal;
in vec3 fragDiffuse;

//light pos
uniform vec3 lightPos;

void main( void ) {
	//light direction
	vec3 LtoF = normalize(lightPos - esFragPos.xyz);
	vec3 normal = normalize(esFragNormal.xyz);
	//diffuse intensity
	float diffuseIntense = dot(LtoF, normal);

	result = vec4(diffuseIntense*fragDiffuse,1);
}