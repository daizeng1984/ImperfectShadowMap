#ifndef __GridNDCDisplay_h__
#define __GridNDCDisplay_h__


#include <GL/glew.h>
#include <stdio.h>
#include "iglu.h"

#include "../PointCloudReader/ISMPointCloud.h"

class GridNDCDisplay : public iglu::IGLUDisplayMode {
public:
    GridNDCDisplay() : IGLUDisplayMode() {}
    virtual ~GridNDCDisplay() {}

    virtual void Initialize( int trackNum );
    virtual void Display();

    virtual const char *GetWindowTitle( void )     
    { 
        return "Multiple Point Cloud Instances."; 
    }

    virtual const char *GetHelpDescriptor( void )  
    { 
        return ""; 
    }

private:

    iglu::IGLUShaderProgram::Ptr  m_shaderProgram;

    // Location of the light and the eye
    iglu::vec3 m_eyePos;

    // Matrices for setting up the view from the eye
    iglu::IGLUMatrix4x4   m_eyeView;
    iglu::IGLUMatrix4x4   m_eyeProj;

    // Matrices for positioning our geometry relative to the world origin
    iglu::IGLUMatrix4x4   m_waveObjModel;
    iglu::IGLUMatrix4x4   m_vertexArrayModel;

    // Point cloud for grided view
    CISMPointCloud* m_vplPointCloud;

};


#endif // __GridNDCDisplay_h__
