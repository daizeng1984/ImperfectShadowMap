// This is the shader that query the RSM(perfect shadow map) and ISM
//    RSM for direct illumination and ISM for indirect illumination.

#version 400
#pragma IGLU_DISABLE BLENDING

out vec4 outputColor;
//input from vertex shader
in vec4 fragPos;
in vec3 fragNormal;
in vec3 fragDiffuse;

//uniform mat4 model;	 // Transforms: model space -> world space

// Common shader funcition for parabolic shadow map projection
#include ../Shaders/Common/common.h.glsl

// N_set is an important parameters that indicates how many VPLs we are
//    gonna use. For N_set = 8, it means we are gonna use 8*8 vpls
uniform int N_set;
// The vpl position buffer and normal buffer, it's texturebuffer that contains
//    vpl position, flux and normal in RGB32F.
uniform samplerBuffer vplBuffer;
// Scale the depth, so it's more clear when we show it
uniform bool scaleDepth = false;
// The near & far planes for projecting the vertex's z-distance, they are >0, so its range is (-n, -f)
uniform float nearDist, farDist;


uniform sampler2D ism;
uniform sampler2D rsmDepth;
uniform sampler2D rsmFlux; //in case we don't need to compute again


// Light transformation matrix, it's normal spot light so simply use perspective projection
uniform mat4 lightView;
uniform mat4 lightProj;
//uniform vec3 lightPos;

uniform float zBias = -0.001;
uniform float areaLightArea = 1.5; // I don't think it's necessary to compute the exact area here for a test so I estimate one
uniform int useFormFactor = 1;
uniform float rsmDepthBias;
uniform float rsmIndirectWeight;
uniform float rsmDirectWeight;
uniform float ismDepthBias;
uniform float maxConeAngle; // In degree! and notice it's half angle used in perspective computation
uniform float lightIntensity = 1.0;
uniform int noVisTest;

void main( void ) {

	vec4 normal = vec4( fragNormal, 0.0 );	
	vec4 diffuseColor = vec4(fragDiffuse, 1);

	//Compute the direct illumination
	// First compute the projected position for light
	vec4 projPos = (lightProj * lightView * fragPos);
	projPos = vec4(projPos.xyz/ projPos.w, 1);
	// Get the refDepth in shadow map
	float refDepth = 2.0*(texture2D(rsmDepth, (projPos.xy + 1)/2.0).r - 0.5 );
	// Compare it with fragment's depth to check if it's in shadow
	float inShadow = refDepth < projPos.z + rsmDepthBias ? 0.0 : 1.0;
	// Compute the cone angle in light view space, so I don't need light Pos and direction
	vec3 lightViewFragPos = (lightView * fragPos).xyz;
	vec3 FtoL = normalize(lightViewFragPos - 0 );
	vec3 LDir = vec3(0,0,-1);
	float inLightCone = dot(FtoL, LDir) > cos(M_Pi*maxConeAngle/180.0) ? 1.0 : 0.0;
	vec4 flux;

	vec3 lightViewNormal = TrasformNormal(lightView, normal.xyz);
	flux = vec4(max(0, dot(-FtoL, lightViewNormal)));
	
	vec4 directIllumin = inShadow * inLightCone * flux * lightIntensity * diffuseColor;

	//Compute one bounced indirect light
	vec4 indirectIllumin = vec4(0);

	// Store the eye-space position of this vertex
	vec4 pos = fragPos;
	
	for(int materialId = 0; materialId < N_set*N_set; ++materialId)
	{
		//vec4 paraCoord = ComputeParabolicCoordinate(pos.xyz, eyePos , at - eyePos, nearDist, farDist);
		vec4 vplPos = texelFetch( vplBuffer, 3*materialId + 0 );
		vec4 vplNorm = texelFetch( vplBuffer, 3*materialId + 1 );  //notice normal is different when tranforming
		vec4 vplFlux = texelFetch( vplBuffer, 3*materialId + 2 ); 

		vec4 paraCoord = ComputeParabolicCoordinate(pos.xyz, vplPos.xyz,  vplNorm.xyz , nearDist, farDist, 0.0001);
		// Pass the negpos value as W to geom shader to discard vertex, since geometry shader is before rasterization
		if (paraCoord.w > 0.0)
		{
			//Compute whether the position is in pos or neg, we ignore neg in geom
			// set back to gl_position
			paraCoord.xy = GridNDC(paraCoord.xy, N_set, materialId);
			float z_query = texture2D(ism, (paraCoord.xy+1.0)/2.0).r;
			if( 2.0*(z_query - 0.5) > paraCoord.z + ismDepthBias || (noVisTest != 0))//not in shadow!
			{
				//if(false)
				//{
					//indirectIllumin += 1.0/(N_set*N_set);
				//}
				//else
				{
					indirectIllumin += vplFlux/(N_set*N_set)*ComputeFormFactor(pos.xyz, normal.xyz, vplPos.xyz, vplNorm.xyz );//paraCoord.z*(1+z_query);
				}
			}
		}
	}

	outputColor = rsmIndirectWeight*diffuseColor*indirectIllumin + rsmDirectWeight*directIllumin;
}