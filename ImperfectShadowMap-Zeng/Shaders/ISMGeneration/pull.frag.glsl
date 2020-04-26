#version 400

//In/Out
in vec2 fragTexCoord;
out vec4 result;

//uniform
uniform sampler2D inputTex;
uniform float width;
uniform float height;
uniform float depthInterval = 0.1;
//It seems that uniform will improve the performance.
vec2 offset[4];

void main( void ) 
{
	//initialize constant offset
	offset[0] = vec2(-0.5/width,  0.5/height);
	offset[1] = vec2(0.5/width,   0.5/height);
	offset[2] = vec2(0.5/width,  -0.5/height);
	offset[3] = vec2(-0.5/width, -0.5/height);

	//Get the finer level's 4 pixels' depth value in [0, 1]
	//And also find the minimum of them
    float finerDepth[4];
	float minDepth = 1.0;
	for( int i = 0; i < 4; ++ i)
	{
		finerDepth[i] = texture2D( inputTex, fragTexCoord.xy + offset[i] ).r;
		minDepth = min(minDepth, finerDepth[i]);
	}
	
	float finalDepth = 0;
	//Accumulate the pixel that close enough to minimum pixel
	int validPixel = 0;
	for( int i = 0; i < 4 ; ++i )
	{
		if ((finerDepth[i] < 1.0) && ( minDepth + depthInterval > finerDepth[i]) )//the pixel's valid and not occluded
		{
			finalDepth += finerDepth[i];
			validPixel += 1;
		}
	}
	

	//Output
	if(validPixel > 0)
	{
		//Average them
		finalDepth = finalDepth/validPixel;
		result = vec4(finalDepth);
	}
	else
	{
		result = vec4(1);
	}

}