#version 400 
#extension GL_EXT_geometry_shader4 : enable

// Tells the OpenGL/IGLU program to enable the depth test when using this shader
#pragma IGLU_ENABLE  DEPTH_TEST

// Information about this geometry shader.  Basically we just abandon the points that
//   is in negative map of the parabolic shadow map
layout (points)		in;
layout (points, max_vertices = 1)		out;

in vec4 verColor[];
out vec4 fragColor;
void main( void )
{
	// If we have vertices in positive paraboloid, pass it to fragment shader, else not
	if ( !(gl_in[0].gl_Position.w < 0.0)  )
	{ 
		gl_Position = vec4(gl_in[0].gl_Position.xyz, 1); 
		fragColor = verColor[0];
		EmitVertex();
	}

	EndPrimitive();	
	
}