#ifndef __SimplePointCloudViewer_h__
#define __SimplePointCloudViewer_h__



#include <GL/glew.h>
#include <stdio.h>
#include "iglu.h"

class SimplePointCloudViewer : public iglu::IGLUDisplayMode {
public:
    SimplePointCloudViewer();
    virtual ~SimplePointCloudViewer() {}

    virtual void Initialize( int trackNum );
    virtual void Display();

    virtual const char *GetWindowTitle( void )     
    { 
        return "Simple Point Cloud Display"; 
    }

    virtual const char *GetHelpDescriptor( void )  
    { 
        return ""; 
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


#endif // __SimplePointCloudViewer_h__
