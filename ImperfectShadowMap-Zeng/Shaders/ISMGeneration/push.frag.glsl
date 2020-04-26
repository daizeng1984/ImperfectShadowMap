#version 400

//In/Out
in vec2 fragTexCoord;
out vec4 result;

//uniform
uniform sampler2D inputTex;
uniform sampler2D finerTex;
uniform float width;
uniform float height;
uniform float depthInterval = 0.1;
//It seems that uniform will improve the performance.
//y| 0 | 1 | 2 |
//|| 3 |   | 4 |
//|| 5 | 6 | 7 |
//+------------> x
vec2 offset[8];
const float weight[8] = float[8](1.0, 1.414, 1.0, 
						  1.414,  1.414,
					      1.0, 1.414, 1.0);    
void main( void ) 
{
	//initialize constant offset
	offset[0] = vec2(-0.5/width,  0.5/height);
	offset[1] = vec2(0,			  0.5/height);
	offset[2] = vec2(0.5/width,   0.5/height);
	offset[3] = vec2(-0.5/width,  0);
	offset[4] = vec2(0.5/width,   0);
	offset[5] = vec2(-0.5/width,  -0.5/height);
	offset[6] = vec2(0,			  -0.5/height);
	offset[7] = vec2(0.5/width,   -0.5/height);
	
	//Get the minimum of depth we computed in pull phase
	float minDepth = texture2D( inputTex, fragTexCoord.xy ).r;
	float finerDepth = texture2D( finerTex, fragTexCoord.xy ).r;
	float finalDepth = 0;
	float totalWeight = 0;
	//If the current pixel is invalid or occluded, interpolate it!
	if ((finerDepth >= 1.0)  || ( minDepth + depthInterval <= finerDepth) )//the pixel's valid and not occluded
	{
		//Interpolate its value from the 8 pixel nearby, ***please make sure the sampling parameter is NEAREST
		for(int i = 0; i < 8; ++i)
		{
			//Get the coarser level's 8 pixels' depth value in [0, 1] if the pixel is valid
			float tempDepth = texture2D( inputTex, fragTexCoord.xy + offset[i]).r; 
			if(tempDepth < 1.0)
			{
				finalDepth += tempDepth*weight[i];
				totalWeight += weight[i];
			}
		}
		
		//Output the result
		if(totalWeight > 0)
		{
			//Average them
			finalDepth = finalDepth/totalWeight;
			result = vec4(finalDepth);
		}
		else
		{
			result = vec4(1);
		}
	
	}
	//else keep it as it is
	else
	{
		result = vec4(finerDepth);
	}

}