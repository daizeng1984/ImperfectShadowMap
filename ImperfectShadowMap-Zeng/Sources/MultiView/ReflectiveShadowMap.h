#ifndef __ReflectiveShadowMap_h__
#define __ReflectiveShadowMap_h__

#include <GL/glew.h>
#include <stdio.h>
#include "iglu.h"

class ReflectiveShadowMap : public iglu::IGLUDisplayMode {
public:
    ReflectiveShadowMap();
    virtual ~ReflectiveShadowMap() {}

    virtual void Initialize( int trackNum );
    virtual void Display();

    virtual const char *GetWindowTitle( void )     
    { 
        return "Reflective Shadow Map: Generate RSM and if possible display it!"; 
    }

    virtual const char *GetHelpDescriptor( void )  
    { 
        return "You can use J,I,K,L,;,/ and middle mouse to move the light camera\n"; 
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


#endif // __ReflectiveShadowMap_h__


