#version 400 
#extension GL_EXT_geometry_shader4 : enable

//#pragma IGLU_ENABLE RASTERIZE_DISCARD

// Geom shader takes in point primitives as input and runs once per primitive,
//   outputting at most one point per input.
layout (points, invocations = 1) in;
layout (points, max_vertices = 1) out;

uniform sampler2D rsmNormal, rsmPos, rsmFlux;
uniform sampler2D randTex2D;
uniform float TEXSAMPLE_LOD = 0.0;
uniform int sampleFlag = 1;
uniform int debugMode = 0;

in int sampleID[];
in vec2 texCoord[];
out vec4 vplFlux;
out vec4 vplNormal;


float Halton1D( unsigned int sampIdx, unsigned int base )
{
	float H=0;
	float m_1_basef = 1.0f / base; 

	for( float mult = m_1_basef ;   
		 sampIdx>0 ; 
		 sampIdx /= base ,        
		 mult *= m_1_basef )                                 
	{
		H += (sampIdx % base) * mult;
	}
	return H;
}

vec2 HaltonSample( int sampIdx )
{
	return vec2( Halton1D( uint(sampIdx), 3u ),
	             Halton1D( uint(sampIdx), 5u ) ); //Set this to avoid the red point
}

vec2 GridSample( int sampIdx )
{
	return texCoord[0];
}

// This takes the random number seed for this pixel and increments it,
//   returning the next pseudorandom number in the sequence
float RandomNumber( inout uint prev )
{
  const uint LCG_A = 1664525u;
  const uint LCG_C = 1013904223u;
  prev = (LCG_A * prev + LCG_C);
  return ( float(prev & 0x00FFFFFF) / float(0x01000000) );
}

vec2 UniformSample( int sampIdx )
{
	uint rndSeed = uint( 0xFFFF *  texture( randTex2D, texCoord[0] ).x );
	return vec2(RandomNumber(rndSeed), RandomNumber(rndSeed));
}
// This is the function adapt from SII authors, they use important sampling based on flux
// this moves 1 sample tap
vec3 adjustSample( vec3 sampleLocation, vec2 texOffset, vec2 texSize, float bias, sampler2D cbsam )
{
	//Make sure they are in texture scope
	const float globalBias = 0.0;
	if ( sampleLocation.x >= texOffset.x && sampleLocation.x <= texOffset.x + texSize.x &&
	     sampleLocation.y >= texOffset.y && sampleLocation.y <= texOffset.y + texSize.y )
     {
		sampleLocation.xy = ( sampleLocation.xy - texOffset ) / texSize.y;

		vec3 A, B, C, D;
		float  flux1, flux2, flux3, flux4, ratio, fA, fB, fC, fD;

		// get the 4 average values of the quadrants
		A = textureLod( cbsam, texOffset + texSize * vec2( 1, 1 ) * 0.25f, globalBias+bias ).rgb;
		B = textureLod( cbsam, texOffset + texSize * vec2( 3, 1 ) * 0.25f, globalBias+bias ).rgb;
		C = textureLod( cbsam, texOffset + texSize * vec2( 1, 3 ) * 0.25f, globalBias+bias ).rgb;
		D = textureLod( cbsam, texOffset + texSize * vec2( 3, 3 ) * 0.25f, globalBias+bias ).rgb;
		
		// and take average brightness
		fA = dot( vec3( 0.299f, 0.587f, 0.114f ), A );
		fB = dot( vec3( 0.299f, 0.587f, 0.114f ), B );
		fC = dot( vec3( 0.299f, 0.587f, 0.114f ), C );
		fD = dot( vec3( 0.299f, 0.587f, 0.114f ), D );

		// adjust sample positions (see ShaderX5 article for details)		
		ratio = ( fA + fC ) / ( fA + fB + fC + fD );
		
		if ( sampleLocation.x < ratio )
		{
			sampleLocation.x = sampleLocation.x / ratio * 0.5f;
			sampleLocation.z *= 0.5f / ratio;
			
				ratio = fA / ( fA + fC );
				
				if ( sampleLocation.y < ratio )
				{
					sampleLocation.y = sampleLocation.y / ratio * 0.5f;
					sampleLocation.z *= 0.5f / ratio;
				}
				else
				{
					sampleLocation.y = 0.5f + ( sampleLocation.y - ratio ) / ( 1.0f - ratio ) * 0.5f;
					sampleLocation.z *= 0.5f / ( 1.0f - ratio );
				}
			
		} else
		{
			sampleLocation.x = 0.5f + ( sampleLocation.x - ratio ) / ( 1.0f - ratio ) * 0.5f;
			sampleLocation.z *= 0.5f / ( 1.0f - ratio );
			
				ratio = fB / ( fB + fD );
				
				if ( sampleLocation.y < ratio )
				{
					sampleLocation.y = sampleLocation.y / ratio * 0.5f;
					sampleLocation.z *= 0.5f / ratio;
				}
				else
				{
					sampleLocation.y = 0.5f + ( sampleLocation.y - ratio ) / ( 1.0f - ratio ) * 0.5f;
					sampleLocation.z *= 0.5f / ( 1.0f - ratio );
				}
			
		}
		sampleLocation.xy = sampleLocation.xy * texSize.x + texOffset.xy;
	}
	return sampleLocation;
}
//Another function modified from Sii sample, z represents importance
#define ADJUST_STEPS	2
vec3 SiiImportantSample(int sampIdx)
{
	vec3 sampleLocation = vec3(HaltonSample( sampIdx ), 1);
	#if (ADJUST_STEPS>=1)
	sampleLocation = adjustSample( sampleLocation, vec2( 0.0, 0.0 ), vec2( 1.0, 1.0 ), 8, rsmFlux );
#endif

	// proceed on more quadrants
#if (ADJUST_STEPS>=2)
	sampleLocation = adjustSample( sampleLocation, vec2( 0.0, 0.0 ), vec2( 0.5, 0.5 ), 7, rsmFlux );
	sampleLocation = adjustSample( sampleLocation, vec2( 0.5, 0.0 ), vec2( 0.5, 0.5 ), 7, rsmFlux );
	sampleLocation = adjustSample( sampleLocation, vec2( 0.0, 0.5 ), vec2( 0.5, 0.5 ), 7, rsmFlux );
	sampleLocation = adjustSample( sampleLocation, vec2( 0.5, 0.5 ), vec2( 0.5, 0.5 ), 7, rsmFlux );
#endif

	// proceed on even more quadrants
#if (ADJUST_STEPS>=3)
	for ( int j = 0; j < 4; j++ )
		for ( int i = 0; i < 4; i++ )
		{
			sampleLocation = adjustSample( sampleLocation, vec2( 0.25 * i, 0.25 * j ), vec2( 0.25, 0.25 ), 6, rsmFlux );
		}
#endif

	// proceed on many many quadrants
#if (ADJUST_STEPS>=4)
	for ( int j = 0; j < 8; j++ )
		for ( int i = 0; i < 8; i++ )
		{
			sampleLocation = adjustSample( sampleLocation, vec2( 0.125 * i, 0.125 * j ), vec2( 0.125, 0.125 ), 5, rsmFlux );
		}
#endif

	// proceed on more quadrants than you want to process ;-)
#if (ADJUST_STEPS>=5)
	for ( int j = 0; j < 16; j++ )
		for ( int i = 0; i < 16; i++ )
		{
			sampleLocation = adjustSample( sampleLocation, vec2( 0.0625 * i, 0.0625 * j ), vec2( 0.0625, 0.0625 ), 4, rsmFlux );
		}
#endif

	return sampleLocation;
}
void main( void )
{
	vec3 sampLoc;
	switch(sampleFlag)
	{
	case 0:
		{ 
			sampLoc = vec3(GridSample( sampleID[0] ), 1);
			break;
		}
	case 1:
		{
			sampLoc = vec3(UniformSample( sampleID[0] ), 1);
			break;
		}
	case 2:
		{
			sampLoc = vec3(HaltonSample( sampleID[0] ), 1);
			break;
		}
	case 3:
		{
			sampLoc = SiiImportantSample( sampleID[0] );
			break;
		}
	default: break;
	}

	
	// Lookup the VPL's 3 vectors in RSM 3 textures
	vec4 rsmTexPos		= textureLod( rsmPos, vec2(sampLoc.x,1.0-sampLoc.y),	TEXSAMPLE_LOD );
	vec4 rsmTexNormal	= textureLod( rsmNormal, vec2(sampLoc.x,1.0-sampLoc.y), TEXSAMPLE_LOD );
	vec4 rsmTexFlux		= textureLod( rsmFlux, vec2(sampLoc.x,1.0-sampLoc.y),	TEXSAMPLE_LOD );
	
	// RSM flux
	vplFlux = vec4( rsmTexFlux.rgb*sampLoc.z, 1 );
	
	// RSM position
	if(debugMode == 1)
	{
		gl_Position = vec4(2.0*(sampLoc - 0.5),1 );
		gl_Position.z = 0;
	}
	else
	{
		gl_Position = vec4(rsmTexPos.xyz, 1);
	}	

	// Normalized normals
	vplNormal = vec4(normalize(rsmTexNormal.xyz), 0);


	EmitVertex();
}