#version 400 

layout(location = IGLU_VERTEX) in vec3 vertex;   // Sends vertex data from models here
layout(location = IGLU_MATERIAL) in int materialId;  //TODO:

uniform mat4 model;	 // Transforms: model space -> world space
uniform mat4 view;   // Transforms: world space -> eye space
uniform mat4 proj;   // Transforms: eye space   -> clip space

out float esFragColor; 
//out vec4 esFragPos;
const int N_set = 2; // means N_set x N_set set

//This function convert any eyespace vector(from any point in the scene to sample position) to 2d paraboloid coordinates
vec2 ReflectedVec3ToParabolicCoordinate(const vec3 v)
{
	vec3 result;
	const vec3 n = vec3(0, 0, -1); //the direction of parabolic shadow map is vec3(0,0,-1)
	result = n + normalize(v);
	result = result/result.z; // x^2 + y^2 = -z 's normal will be (x, y, 1)
	return result.xy;
}

//build the onb for normal, you just need to input normal and you don't need to normalize them
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

//Compute parabolic coordinate of any position in scene based on sample point and sample direction,or VPL's position and direction.
//   This function also return the z-perspective value and positiveness of position based on nearDist and farDist which is also positive
//   rather than negative, the w component of the return value +1 represent the positive and -1 represent negative 
vec4 ComputeParabolicCoordinate(const vec3 position, const vec3 samplePoint, const vec3 sampleDir, float nearDist, float farDist)
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
	float z_persp = ((nearDist+farDist)*-dist + (2*nearDist*farDist))/(dist*(nearDist-farDist));
	//Compute positiveness
	float w_posneg = dot( worldVec, sampleDir) > 0 ? 1 : -1;

	return vec4(paraCoord, z_persp, w_posneg);
}

//convert NDC in [-1 , 1] to a grid block in [-1, 1], 
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


// Information about the near & far planes for projecting the vertex's z-distance
const float nearDist = 0.1, farDist = 5.0;
void main( void )
{  
	// Store the eye-space position of this vertex, send to frag shader
	//esFragPos = view * model * vec4( vertex, 1.0f );
	esFragColor = materialId==materialId? 1: 0.1;
	
	vec4 pos = model * vec4( vertex, 1.0f );

	gl_Position = proj * ( view * model * vec4( vertex, 1.0f ));	

	vec3 eyePos = vec3(0.0,0.0,1.0);
	vec3 n[4];
	n[0] = vec3(0, 0, 1);
	n[1] = vec3(0, 0, -1);
	n[2] = vec3(0, 1, 0);
	n[3] = vec3(0, -1, 0);

	vec4 paraCoord = ComputeParabolicCoordinate(pos.xyz, eyePos, n[materialId], nearDist, farDist);
	//gl_Position = vec4(paraCoord.xyz, 1.0);

	//Compute whether the position is in pos or neg, we ignore neg in geom
	//if( paraCoord.w < 0)
	//{
		////neg
		//esFragColor = 0.0;
	//}
	// set back to gl_position
	vec2 xy = GridNDC(gl_Position.xy/gl_Position.w, N_set, materialId);
	gl_Position.xy = xy*gl_Position.w;
}