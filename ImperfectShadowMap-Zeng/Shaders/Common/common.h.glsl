#ifndef __common_h_glsl__
#define __common_h_glsl__

const float M_Pi = 3.14159265358979323f;

//This function convert any eyespace vector(from any point in the scene to sample position) to 2d paraboloid coordinates
vec2 ReflectedVec3ToParabolicCoordinate(const vec3 v)
{
	vec3 result;
	const vec3 n = vec3(0, 0, -1); //the direction of parabolic shadow map is vec3(0,0,-1)
	result = n + normalize(v);
	result = result/result.z; // x^2 + y^2 = -z 's normal will be (x, y, 1)
	return result.xy;
}

//build the onb for normal (world coordinate), you just need to input normal and you don't need to normalize them
void Onb(inout vec3 normal, out vec3 outBinormal, out vec3 outTangent )
{
	normal = normalize(normal);
    if( abs(normal.x) > abs(normal.z) )
    {
    outBinormal.x = -normal.y;
    outBinormal.y =  normal.x;
    outBinormal.z =  0;
    }
    else
    {
    outBinormal.x =  0;
    outBinormal.y = -normal.z;
    outBinormal.z =  normal.y;
    }

    outBinormal = normalize(outBinormal);
    outTangent = cross( outBinormal, normal );
}
//convert Orthorgonal normal basis's vector to world coordinate
vec3 Onb2World(const vec3 onVec3, const vec3 normal, const vec3 binormal, const vec3 tangent)
{
	return onVec3.x*tangent + onVec3.y*binormal + onVec3.z*normal;
}

//convert world coordinates to ONB coordiante
vec3 World2Onb(const vec3 world, const vec3 normal, const vec3 binormal, const vec3 tangent)
{
	return vec3( dot(world, tangent), dot( world, binormal), dot(world, normal));
}

//Compute parabolic coordinate of any position (first argument) in scene based on parabolic sample point(2nd argument) and sample 
//   direction (3rd argument, you are responsible to normalize it),or VPL's position and direction.
//   This function also return the z-perspective value and positiveness of position based on nearDist and farDist which is also positive
//   rather than negative, the w component of the return value +1 represent the positive and -1 represent negative 
//   bias is for eliminating query artifact when light is close to be vertical to the normal
vec4 ComputeParabolicCoordinate(const vec3 position, const vec3 samplePoint, const vec3 sampleDir, float nearDist, float farDist, float bias = 0.0)
{
	//build onb based on sample direction
	vec3 binormal, tangent;
	vec3 normal = -sampleDir;
	Onb(normal, binormal, tangent);

	//world vector of pos-eye or pos - samplepoint
	vec3 worldVec = position - samplePoint;
	//Compute the parabolic coordinates assuming normal = (0, 0, -1), so we convert from world to Onb coordinate
	vec2 paraCoord = ReflectedVec3ToParabolicCoordinate(World2Onb(worldVec, normal, binormal, tangent));
	//Compute the z perspective distance based on near/far distance
	float dist = length( worldVec);
	float z_persp = ((nearDist+farDist)*-dist + (2*nearDist*farDist))/(dist*(nearDist-farDist)); //near far is positive. but z_e = -dist < 0 
	// notice that n -> -1  f -> 1, so in fix pipeline, it'll convert the [-1, 1] to [0, 1] and this range is decided in glDepthRange() function
	//Compute positiveness
	float w_posneg = dot( worldVec, sampleDir) > bias ? 1 : -1;

	return vec4(paraCoord, z_persp, w_posneg);
}

//convert NDC in [-1 , 1] to a grid block in [-1, 1], *make sure* your input xy coordinate is already divided by w
vec2 GridNDC(vec2 xy, int rowNum, int gridId)
{
	// get the transformed position based on Id /in [0,1,..., N_set x N_set]
	float xBlockSize = 2.0/rowNum;
	float yBlockSize = 2.0/rowNum;
	// get x, y index based on material id
	int xIdx = gridId%rowNum;
	int yIdx = gridId/rowNum;
	// compute the transform
	xy = yBlockSize*(xy + vec2(1, 1))/2.0;
	xy = xy + vec2(xBlockSize*xIdx, yBlockSize*yIdx);
	xy = xy - vec2(1,1);
	return xy;
}

vec3 GetVPLSample(sampler2D sampler, int matId, int width, float lod = 0)
{
	int i = matId%width;
	int j = matId/width;
	vec2 sampleLoc = vec2(i/float(width), j/float(width));
	return textureLod(sampler, sampleLoc, lod).xyz;
}

float ComputeFormFactor(vec3 receiverPos, vec3 receiverNormal, vec3 senderPos, vec3 senderNormal)
{
	//
	vec3 dis = receiverPos - senderPos;
	return max(0, dot(-dis, receiverNormal)) * max(0, dot(dis, senderNormal ))/ dot(dis,dis);
}

vec3 TrasformNormal(mat4 transform, vec3 normal)
{
	return normalize((inverse( transpose( transform ) ) * vec4( normal, 0.0 )).xyz);
}
#endif