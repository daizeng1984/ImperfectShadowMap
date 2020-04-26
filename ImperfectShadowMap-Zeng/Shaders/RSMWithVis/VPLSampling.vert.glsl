// The traversal vertex shader is simple, and just passes the input
//    down to the geometry shader so it can be processed.

#version 400

in vec2 vertex;

out int sampleID;
out vec2 texCoord;

void main( void )
{  
	sampleID = gl_VertexID;
	texCoord = vertex;
}