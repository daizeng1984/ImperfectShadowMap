#ifndef __GlobalData_h__
#define __GlobalData_h__


#include "iglu.h"
#include "../Camera/MyCamera.h"
#include "../PointCloudReader/ISMPointCloud.h"
#include "../Sampler/Sampler.h"

class CPointCloudReader;
class CISMPointCloud;
class CMyCamera;

using namespace iglu;

//data
struct SGlobalData
{
    //The scene geometry 5 wall and 2 box
    char * cornellFileName;
    IGLUOBJReader::Ptr      cornellObj;
    CPointCloudReader*      cornellPtCloud;
    CISMPointCloud*         cornellMultipleVplPCloud;
    //Light geometry
    char * sphereFileName;
    IGLUOBJReader::Ptr      sphereObj;
    CPointCloudReader*      spherePtCloud;
    CISMPointCloud*         sphereMultipleVplPCloud;
    
    char * arrowFileName;
    IGLUOBJReader::Ptr      arrowObj;

    //Windows
    IGLUMultiDisplayWindow::Ptr         myWin;
    //Trackball for interaction
    IGLUTrackball::Ptr		trackBall;
    //Camera
    CMyCamera::Ptr          camera;

    //Simple view 
    IGLUShaderProgram::Ptr  simpleShader;
    IGLUShaderProgram::Ptr  lambertShader;

    //RSM shader and fbo
    enum {RSM_SIZE = 512}; //enum hack
    IGLUShaderProgram::Ptr  rsmShader;
    IGLUShaderProgram::Ptr  rsmWithVisQueryShader;
    IGLUFramebuffer::Ptr rsmFbo; //3MRT
    enum {RSM_FLUX = IGLU_COLOR0, RSM_WORLD_POS = IGLU_COLOR1, RSM_WORLD_NORMAL = IGLU_COLOR2, RSM_TYPE_NUM = 4};
    uint currentRsmType;

    // Light related
    float lightNear;
    float lightFar;
    float lightMaxAngle;
    float lightIntensity;
    CMyCamera::Ptr lightCamera;
    IGLUMatrix4x4 lightView;
    IGLUMatrix4x4 lightProj;

    //ISM shader and fboo
    enum {SHADOWMAP_SIZE = 1024, POINT_NUMBER_EACH_INSTANCE = 5000, SHADOWMAP_MIP_MAP_LEVEL = 8};
    enum {MAX_N_SET = 64};
    IGLUShaderProgram::Ptr  ismGenShader;
    IGLUShaderProgram::Ptr  ismQueryShader;
    IGLUFramebuffer::Ptr    ismGenFbo;
    IGLURenderTexture::Ptr  ismGenTex;

    //ISM sampling
    IGLUVertexArray::Ptr garbagePoints;
    IGLUTransformFeedback::Ptr vplTransformfeeback;
    IGLUBuffer::Ptr vplRawBuffer;
    IGLUTextureBuffer::Ptr vplTexBuffer;
    IGLUShaderProgram::Ptr vplSamplingShader;
    IGLURandomTexture2D::Ptr randTex2D;
    

    //VPL drawing
    IGLUShaderProgram::Ptr vplDrawingShader;

    //Pull and push
    CSampler::Ptr pPullSamplerArray[SHADOWMAP_MIP_MAP_LEVEL];
    CSampler::Ptr pPushSamplerArray[SHADOWMAP_MIP_MAP_LEVEL];
    IGLURenderTexture::Ptr pullTexArray[SHADOWMAP_MIP_MAP_LEVEL];
    IGLURenderTexture::Ptr pushTexArray[SHADOWMAP_MIP_MAP_LEVEL];


    // Model View Proj
    IGLUMatrix4x4 proj;

    // UI parameters
    IGLUFloat rsmDepthBias;
    IGLUFloat rsmIndirectWeight;
    IGLUFloat rsmDirectWeight;
    IGLUFloat ismDepthBias;
    IGLUInt pullPushLevel;
    IGLUFloat pullPushInterval;
    IGLUFloat rsmLightIntensity;
    IGLUFloat rsmLightConeAngle;
    IGLUInt N_SET;
    IGLUBool rsmVisTest;
    IGLUBool ismDrawISM;
    IGLUBool showVPLs;
    IGLUInt sampleFlag;

    SGlobalData()
    {
        //memset(this,0, sizeof(SGlobalData)); since we add magic ptr we no longer safe to do this
        
    }
};

//Functions
// Code that initializes our OpenGL state.  This is guaranteed (by IGLUWindow) to be 
//    called after the OpenGL context & all extensions have been initialized
void OpenGLInitialization( void );

// Track any updates to the trackball matrix when the mouse moves 
void Motion(int x, int y);

// When the user clicks/releases in the main window, start/stop tracking with our trackball
void Button(int button, int state, int x, int y  );


void Keys(unsigned char key, int x, int y);


// The function that generate RSM to 3MRT in rsmFbo
void GenerateRSM();

void GenerateISM( iglu::IGLUTextureBuffer::Ptr vplPosBuffer );


void PullPushPhase(iglu::IGLURenderTexture& srcTex, int maxPullPushLevel, float depthInterval = 0.0);

void DrawMainLight();
void DrawVPLs(iglu::IGLUTextureBuffer::Ptr vplBuffer, iglu::IGLUOBJReader::Ptr lightObj);

void ForwardQueryRSMwISM( iglu::IGLUTextureBuffer::Ptr vplPosBuffer, iglu::IGLUTexture& ism, iglu::IGLUFramebuffer::Ptr rsmFBo);

void DeferredQueryRSM();

void GridSamplePointBuffer(float * buffer, unsigned int bufferLength);

void SampleVPLs(iglu::IGLUFramebuffer::Ptr rsmFbo, bool outputTexSamplePoints = false);

#endif // __GlobalData_h__