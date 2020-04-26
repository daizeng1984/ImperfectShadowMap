#ifndef __RSMWithVisibility_h__
#define __RSMWithVisibility_h__

#include <GL/glew.h>
#include <stdio.h>
#include "iglu.h"

class RSMWithVisibility : public iglu::IGLUDisplayMode {
public:
    RSMWithVisibility();
    virtual ~RSMWithVisibility() {}

    virtual void Initialize( int trackNum );
    virtual void Display();

    virtual const char *GetWindowTitle( void )     
    { 
        return "RSM with ISM for visibility test."; 
    }

    virtual const char *GetHelpDescriptor( void )  
    { 
        return "You can use right mouse to control the camera.\n"
            "Keys A,W,S,D,F,V to move the camera.\n"
            "Middle mouse to move the light around.\n"
            ; 
    }

private:
    iglu::IGLUShaderProgram::Ptr  m_shaderProgram;
    // Location of the light and the
    iglu::vec3            m_eyePos;

    // Matrices for setting up the v
    iglu::IGLUMatrix4x4   m_eyeView;
    iglu::IGLUMatrix4x4   m_eyeProj;

    // Matrices for positioning our 
    iglu::IGLUMatrix4x4   m_waveObjModel;
    iglu::IGLUMatrix4x4   m_vertexArrayModel;
};


#endif // __RSMWithVisibility_h__

