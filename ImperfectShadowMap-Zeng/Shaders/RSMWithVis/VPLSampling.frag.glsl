// This should never be executed....  Geometry should be discarded via:  
//     glEnable( GL_RASTERIZER_DISCARD ); 
//
// Basically this just needs to be a simple, syntactically correct shader
//     so the GLSL compiler doesn't complain.  Effectively this is a no-op.

#version 400

in vec4 vplFlux;
in vec4 vplNormal;
out vec4 fragOut;

void main( void )
{
    fragOut = vec4(1,0,1,1); 
}
