#version 400

//In/Out
in vec2 fragTexCoord;
out vec4 result;

//uniform
uniform sampler2D inputTex;
uniform float width;
uniform float height;

void main( void ) 
{
	float depth = texture2D(inputTex, fragTexCoord.xy).r;
	result = vec4(pow(depth, 32));

}