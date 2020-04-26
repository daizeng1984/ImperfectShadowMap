#ifndef __ImperfectShadowMapTest_h__
#define __ImperfectShadowMapTest_h__


#include <GL/glew.h>
#include <stdio.h>
#include "iglu.h"

#include "../PointCloudReader/ISMPointCloud.h"
#include "../Sampler/Sampler.h"

class ISMAreaLightViewer : public iglu::IGLUDisplayMode {
public:
    ISMAreaLightViewer() : IGLUDisplayMode() {}
    virtual ~ISMAreaLightViewer() {}

    virtual void Initialize( int trackNum );
    virtual void Display();

    virtual const char *GetWindowTitle( void )     
    { 
        return "Imperfect Shadow Map For Area Light."; 
    }

    virtual const char *GetHelpDescriptor( void )  
    { 
        return ""; 
    }

private:
    void PullPushPhase();
    void UpdateModelView();
    void InitializeModelView();

    // Create texture buffer based on the hard coded light position
    // Given width and height
    void CreateAreaLightBuffer(int width, int height);

private:
    iglu::IGLUBuffer::Ptr         vplPosbuffer;
    iglu::IGLUTextureBuffer::Ptr  vplPosTexBuffer;
    iglu::IGLUBuffer::Ptr         vplNormbuffer;
    iglu::IGLUTextureBuffer::Ptr  vplNormTexBuffer;

private:

    iglu::IGLUShaderProgram::Ptr  m_shaderProgram;

    // Location of the light and the eye
    iglu::vec3 m_eyePos;

    // Matrices for setting up the view from the eye
    iglu::IGLUMatrix4x4   m_eyeView;
    iglu::IGLUMatrix4x4   m_eyeProj;

    // Matrices for positioning our geometry relative to the world origin
    iglu::IGLUMatrix4x4   m_waveObjModel;
    iglu::IGLUMatrix4x4   m_lightModel;

};


#endif // __ImperfectShadowMapTest_h__
