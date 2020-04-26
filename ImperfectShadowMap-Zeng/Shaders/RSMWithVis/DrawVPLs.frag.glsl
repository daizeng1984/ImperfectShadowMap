// This is a shader simple view
#version 400
#pragma IGLU_ENABLE  BLENDING

out vec4 result;
in vec4 color;

void main( void ) {
	result = color;
}