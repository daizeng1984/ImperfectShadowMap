//Simply output whatever color
#version 400
out vec4 result;
#pragma IGLU_DISABLE BLENDING
in vec4 fragColor;
void main( void ) {
	result = fragColor;
}