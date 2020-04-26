// This is a shader simple view

#version 400
out vec4 result;
uniform vec4 color;  // color of the vertex

void main( void ) {
	result = color;
}