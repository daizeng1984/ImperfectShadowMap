//Geometry shader to extend point to rectangle
#version 400 
#extension GL_EXT_geometry_shader4 : enable

// Tells the OpenGL/IGLU program to enable the depth test when using this shader
#pragma IGLU_ENABLE  DEPTH_TEST

// Information about this geometry shader.  It's called only once on each
//   input triangle, and creates a set of triangle_strip (really, triangles) 
//   with up to 6 vertices output for each input triangle.
layout (points)       in;
layout (triangle_strip, max_vertices = 4)	out;
uniform float w = 0.5/400.0; //as box width or square width for each point
uniform float maxRadius = 1; //as box width or square width for each point

// Data passed down from the vertex shader
in vec4 esFragPos[];
out vec4 geomDepth;

void main( void )
{
	vec4 offset[4];
	offset[0] = vec4( -w/2.0, -w/2.0, 0, 0 );
	offset[1] = vec4( w/2.0, -w/2.0, 0, 0 );
	offset[2] = vec4( -w/2.0, w/2.0, 0, 0 );
	offset[3] = vec4( w/2.0, w/2.0, 0, 0 ); // CCW begin from the bottom left point

	//const unrollable loop
	for(int i = 0; i < 4; ++i)
	{
		gl_Position = gl_in[0].gl_Position + offset[i]*gl_in[0].gl_Position.w ;
		geomDepth = vec4(max(0.5, length(esFragPos[0].xyz)/maxRadius));
		EmitVertex();
	}

	EndPrimitive();
}