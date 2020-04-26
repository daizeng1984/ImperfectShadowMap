// This is a shader to "create" a parabolic shadowmap, notice that
//	the environment geometry has to be sufficiently tessellated

#version 400
#pragma IGLU_DISABLE BLENDING

out vec4 outputColor;
in vec4 fragPos;
in vec3 fragNormal;
uniform mat4 model;	 // Transforms: model space -> world space

uniform int N_set; // means N_set x N_set set
uniform samplerBuffer vplPosBuffer; //light pos
uniform samplerBuffer vplNormBuffer; //light direction

uniform float zBias = 0.001;
uniform sampler2D ism;

#include ../Shaders/Common/common.h.glsl

uniform float areaLightArea = 1.5; // I don't think it's necessary to compute the exact area here for a test so I estimate one
uniform int useFormFactor = 1;
// Information about the near & far planes for projecting the vertex's z-distance
const float nearDist = 0.1, farDist = 20.0;


void main( void ) {
	// Get the vector from esPos to eye, the compute the coordinate for front or back
	float intensity = 0;

	// Store the eye-space position of this vertex
	vec4 pos = model * fragPos;
	vec4 normal = inverse( transpose( model ) ) * vec4( fragNormal, 0.0 );	
	
	for(int materialId = 0; materialId < N_set*N_set; ++materialId)
	{
		//vec4 paraCoord = ComputeParabolicCoordinate(pos.xyz, eyePos , at - eyePos, nearDist, farDist);
		vec4 vplPos = vec4( texelFetch(vplPosBuffer, materialId).xyz, 1);
		vec4 vplNorm = vec4( texelFetch(vplNormBuffer, materialId).xyz, 1); //notice normal is different when tranforming

		vec4 paraCoord = ComputeParabolicCoordinate(pos.xyz, vplPos.xyz,  vplNorm.xyz , nearDist, farDist);
		// Pass the negpos value as W to geom shader to discard vertex, since geometry shader is before rasterization
		if (paraCoord.w > 0)
		{
			//Compute whether the position is in pos or neg, we ignore neg in geom
			// set back to gl_position
			paraCoord.xy = GridNDC(paraCoord.xy, N_set, materialId);
			float z_query = texture2D(ism, (paraCoord.xy+1.0)/2.0).r;
			if( 2.0*(z_query - 0.5) > paraCoord.z + zBias)//not in shadow!
			{
				if(useFormFactor == 0)
				{
					intensity += 1.0/(N_set*N_set);
				}
				else
				{
					intensity += areaLightArea/(N_set*N_set) * ComputeFormFactor(pos.xyz, normal.xyz, vplPos.xyz, vplNorm.xyz );//paraCoord.z*(1+z_query);
				}
			}
		}
	}

	outputColor = vec4(intensity);
	outputColor.w = 1;
}