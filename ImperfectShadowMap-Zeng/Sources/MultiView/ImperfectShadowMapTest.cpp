#include "ImperfectShadowMapTest.h"
#include "../GlobalData/GlobalData.h"
using namespace iglu;

extern SGlobalData* g_data;

//The scene geometry consists of two objects: a wavefront obj file and a vertex array for square


// Create a shadow map.  Make sure to use nearest neighbor on the z-buffer/shadow map.  
IGLUFramebuffer::Ptr    vplShadowMap = 0;
const uint SHADOWMAP_SIZE = 1024;
const uint N_SET = 8;
const uint SHADOWMAP_MIP_MAP_LEVEL = 8;
const uint POINT_NUMBER_EACH_INSTANCE = 5000;

IGLUShaderProgram::Ptr ismShaderQuery = 0;

IGLUBool showPushTex(false, "Show pull/pushed texture");
IGLUBool usePullPush(true, "Use Pull/Push phase");
IGLUBool useFormFactor(true, "Use form factor in shadow query");
IGLUBool useLinearInterpolation(false, "Linear texturing for shadow query");
IGLUFloat zBias( -0.005, IGLURange<float>(-0.02,0), 0.0002, "Shadow map bias" );
IGLUFloat lightTranslationX( 0, IGLURange<float>(-3,3), 0.0002, "Move light along X axis" );
IGLUFloat lightTranslationZ( 0, IGLURange<float>(-3,3), 0.0002, "Move light along Z axis" );

const float MIN_DEPTH_INTERVAL = 1.0/10e6;
IGLUFloat depthInterval( MIN_DEPTH_INTERVAL, IGLURange<float>(MIN_DEPTH_INTERVAL,0.02), MIN_DEPTH_INTERVAL, "Depth interval" );
IGLUInt maxPullPushLevel( 2, IGLURange<int>(2, SHADOWMAP_MIP_MAP_LEVEL-1), 1, "Max allowed push texture Level" );


CSampler::Ptr pPullSamplerArray[SHADOWMAP_MIP_MAP_LEVEL] = {0};
CSampler::Ptr pPushSamplerArray[SHADOWMAP_MIP_MAP_LEVEL] = {0};
IGLURenderTexture::Ptr pullTexArray[SHADOWMAP_MIP_MAP_LEVEL] = {0};
IGLURenderTexture::Ptr pushTexArray[SHADOWMAP_MIP_MAP_LEVEL] = {0};

CSampler::Ptr pDepthRescaler = 0;
IGLURenderTexture::Ptr rescaledDepth = 0;

void ISMAreaLightViewer::Initialize( int trackNum )
{
    //Create Ui
    g_data->myWin->AddWidget(trackNum,&showPushTex);
    g_data->myWin->AddWidget(trackNum,&useFormFactor);
    g_data->myWin->AddWidget(trackNum,&usePullPush);
    g_data->myWin->AddWidget(trackNum,&zBias);
    g_data->myWin->AddWidget(trackNum,&depthInterval);
    g_data->myWin->AddWidget(trackNum,&maxPullPushLevel);
    g_data->myWin->AddWidget(trackNum,&lightTranslationX);
    g_data->myWin->AddWidget(trackNum,&lightTranslationZ);

    InitializeModelView();    
    
    CreateAreaLightBuffer(N_SET, N_SET);

    
    // Load a shader that queries a shadow map
    m_shaderProgram = 0;
    m_shaderProgram = new IGLUShaderProgram("../Shaders/ISMGeneration/ISMGeneration.vert.glsl",
        "../Shaders/ISMGeneration/ISMGeneration.geom.glsl", 
        "../Shaders/ISMGeneration/ISMGeneration.frag.glsl");
    m_shaderProgram->SetProgramEnables(IGLU_GLSL_DEPTH_TEST);

    // Set the program texture uniform
    m_shaderProgram["vplPosBuffer"] = vplPosTexBuffer;
    m_shaderProgram["vplNormBuffer"] = vplNormTexBuffer;
    m_shaderProgram["N_set"] = N_SET;

    // Set the FBO
    vplShadowMap = IGLUFramebuffer::Create(GL_RGBA8, SHADOWMAP_SIZE, SHADOWMAP_SIZE, true, false);

    //Shadow query
    ismShaderQuery = new IGLUShaderProgram("../Shaders/ISMGeneration/ISMQuery.vert.glsl", 
        "../Shaders/ISMGeneration/ISMQuery.frag.glsl");
    ismShaderQuery->SetProgramEnables(IGLU_GLSL_DEPTH_TEST);
    ismShaderQuery["proj"] = m_eyeProj;
    ismShaderQuery["vplPosBuffer"] = vplPosTexBuffer;
    ismShaderQuery["vplNormBuffer"] = vplNormTexBuffer;
    ismShaderQuery["N_set"] = N_SET;

    //Create the pull push hierachies
    for (int i = 0; i < SHADOWMAP_MIP_MAP_LEVEL; ++ i)
    {
        pullTexArray[i] = new IGLURenderTexture2D( SHADOWMAP_SIZE/int(powf(2,i))
            , SHADOWMAP_SIZE/int(powf(2,i)), GL_RGB32F ); //32 bit depth info only, but red is not good to see the depth map

        pPullSamplerArray[i] = new CSampler("../Shaders/ISMGeneration/pull.frag.glsl", pullTexArray[i]);

        pushTexArray[i] = new IGLURenderTexture2D( SHADOWMAP_SIZE/int(powf(2,i))
            , SHADOWMAP_SIZE/int(powf(2,i)), GL_RGB32F ); //32 bit depth info only

        pPushSamplerArray[i] = new CSampler("../Shaders/ISMGeneration/push.frag.glsl", pushTexArray[i]);
    }

    rescaledDepth = new IGLURenderTexture2D(SHADOWMAP_SIZE, SHADOWMAP_SIZE, GL_RGB32F);
    pDepthRescaler = new CSampler("../Shaders/ISMGeneration/depthRescaler.frag.glsl", rescaledDepth);
}

void ISMAreaLightViewer::Display()
{
    //Update the necessary matrix
    UpdateModelView();

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Generate ISM
    glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
    vplShadowMap->Bind();
    vplShadowMap->Clear();
        m_shaderProgram->Enable(); 
        m_shaderProgram["model"] = m_waveObjModel;
        g_data->cornellPtCloud->Draw(m_shaderProgram);
        m_shaderProgram->Disable(); 
    vplShadowMap->Unbind();
    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

    // Show ISM in different way
    if (!showPushTex)
    {
        //Pull and Push and save the texture to pushTexArray[0]
        if (usePullPush)
        {
            PullPushPhase();
            ismShaderQuery["ism"] = pushTexArray[0];
        }
        else
        {
            ismShaderQuery["ism"] = vplShadowMap[IGLU_DEPTH];
        }

        // Draw with shadow
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        ismShaderQuery->Enable();
        ismShaderQuery["view"] = m_eyeView;
        ismShaderQuery["model"] = m_waveObjModel;
        ismShaderQuery["zBias"]      = zBias;
        ismShaderQuery["useFormFactor"]  = (int)useFormFactor;
        g_data->cornellObj->Draw(ismShaderQuery);
        //glFrontFace(GL_CCW)
        ismShaderQuery->Disable();

    }
    else
    {
        if (usePullPush)
        {
            PullPushPhase();
            //Rescale final texture
            pDepthRescaler->SampleFrom(pushTexArray[0], false);
        }
        else
        {
            //Rescale final texture
            pDepthRescaler->SampleFrom(&vplShadowMap[IGLU_DEPTH], false);
        }

        rescaledDepth->SetTextureParameters( IGLU_MAG_NEAREST | IGLU_MIN_NEAREST );
        IGLUDraw::Fullscreen(rescaledDepth);
        rescaledDepth->SetTextureParameters( IGLU_MAG_LINEAR | IGLU_MIN_LINEAR );
    }
    //Display point number
    char text[255];
    sprintf(text, "Point Number is: %d",POINT_NUMBER_EACH_INSTANCE);
    IGLUDraw::DrawTextW(IGLU_FONT_VARIABLE, 0, 20, text);

}

void ISMAreaLightViewer::PullPushPhase()
{
    //Pull and Push and save the texture to pushTexArray[0]
    pPullSamplerArray[0]->CopyFrom(&vplShadowMap[IGLU_DEPTH]);
    for (int i = 0; i < maxPullPushLevel -1 ; ++ i)
    {
        pPullSamplerArray[i+1]["depthInterval"] = depthInterval;
        pPullSamplerArray[i+1]["depthInterval"] = depthInterval;
        pPullSamplerArray[i+1]->SampleFrom(pullTexArray[i]);
    }

    //Copy the coarsest
    pPushSamplerArray[maxPullPushLevel-1]->CopyFrom(pullTexArray[maxPullPushLevel-1]);
    for (int i = maxPullPushLevel -1; i > 0  ; -- i)
    {
        pPushSamplerArray[i-1]["finerTex"] = pullTexArray[i-1];
        pPushSamplerArray[i-1]["depthInterval"] = depthInterval;
        pPushSamplerArray[i-1]->SampleFrom(pushTexArray[i]);
    }
}

void ISMAreaLightViewer::UpdateModelView()
{
    //Camera change
    m_eyeView = g_data->camera->GetMatrix();
    //Object model change due to trackball
    //m_waveObjModel   = g_data->trackBall->GetMatrix();
    //light translation
    m_lightModel = IGLUMatrix4x4::Translate( lightTranslationX *vec3::XAxis()) 
        * IGLUMatrix4x4::Translate( lightTranslationZ*vec3::ZAxis())* m_waveObjModel;
}

void ISMAreaLightViewer::InitializeModelView()
{
    // Location of the light and the eye
    m_eyePos   = vec3(2.78, 2.73, -8.00);

    // Matrices for setting up the view from the eye
    //m_eyeView = IGLUMatrix4x4::LookAt( m_eyePos, vec3(2.78, 2.73, 2.795), vec3::YAxis() );
    g_data->camera->SetView(m_eyePos, vec3(2.78, 2.73, 2.795), vec3::YAxis());

    //TODO: refactor shader's far near plane value.
    m_eyeProj = IGLUMatrix4x4::Perspective( 40, 1, 0.1, 20.0 );

    // Matrices for positioning our geometry relative to the world origin
    m_waveObjModel   = g_data->trackBall->GetMatrix();
    m_lightModel = m_waveObjModel;//Translate( -0.45*vec3::YAxis() );
}

void ISMAreaLightViewer::CreateAreaLightBuffer( int width, int height )
{
    //Initial pointer
    vplPosbuffer = 0;
    vplPosTexBuffer = 0;
    vplNormbuffer = 0;
    vplNormTexBuffer = 0;

    //# Vertex list
    //# light for vertex
    //        v1 2.130 5.487 2.270
    //        v2 2.130 5.487 3.320
    //        v3 3.430 5.487 3.320
    //        v4 3.430 5.487 2.270
    vec3 v1( 2.130, 5.487, 2.270 );
    vec3 v2( 2.130, 5.487, 3.320 );
    vec3 v3( 3.430, 5.487, 3.320 );
    vec3 v4( 3.430, 5.487, 2.270 );

    // Create the float buffer for vpl position
    float* vplPosFloatData = new float[width*height*3]; //RGB32F
    int Id = 0;
    // For each pixel fill the interpolated position
    // Notice this position is pre model
    for (int i = 0; i < width; ++i)
    {
        for ( int j = 0; j < height; ++j)
        {
            //texcoord x,y
            float x = (Id%width + 0.5)/float(width);
            float y = (Id/height + 0.5)/float(height);
            assert( 0 < x && x < 1.0 && 0 < y && y < 1.0);

            //interpolate the 4 vertices of the rectangle
            vec3 tempV = y*(x*v1 + (1-x)*v4) + (1-y)*(x*v2 + (1-x)*v3);
            vplPosFloatData[Id*3 + 0] = tempV.X();
            vplPosFloatData[Id*3 + 1] = tempV.Y();
            vplPosFloatData[Id*3 + 2] = tempV.Z();

            ++Id;
        }
    }

    //For vpl normal buffer I just duplicate this logic
    vec3 n1( 0, -1, 0 );
    vec3 n2( 0, -1, 0 );
    vec3 n3( 0, -1, 0 );
    vec3 n4( 0, -1, 0 );

    // Create the float buffer for vpl position
    float* vplNormFloatData = new float[width*height*3]; //RGB32F
    // Create float buffer for
    Id = 0;
    for (int i = 0; i < width; ++i)
    {
        for ( int j = 0; j < height; ++j)
        {
            //texcoord x,y
            float x = (Id%width + 0.5)/float(width);
            float y = (Id/height + 0.5)/float(height);
            assert( 0 < x && x < 1.0 && 0 < y && y < 1.0);

            //interpolate the 4 vertices of the rectangle
            vec3 tempV = y*(x*n1 + (1-x)*n4) + (1-y)*(x*n2 + (1-x)*n3);
            vplNormFloatData[Id*3 + 0] = tempV.X();
            vplNormFloatData[Id*3 + 1] = tempV.Y();
            vplNormFloatData[Id*3 + 2] = tempV.Z();

            ++Id;
        }
    }
    // Do we need a scalar? No. For RGB32F
    vplPosbuffer = new IGLUBuffer();
    vplPosbuffer->SetBufferData( sizeof( float )*width*height*3, vplPosFloatData, IGLU_STATIC|IGLU_DRAW );
    vplNormbuffer = new IGLUBuffer();
    vplNormbuffer->SetBufferData( sizeof( float )*width*height*3, vplNormFloatData, IGLU_STATIC|IGLU_DRAW );

    // Create a texture buffer to pass light
    vplPosTexBuffer = new IGLUTextureBuffer();
    vplPosTexBuffer->BindBuffer( GL_RGB32F, vplPosbuffer ); //when you set this format you dont' need to worry about the clamp
    vplNormTexBuffer = new IGLUTextureBuffer();
    vplNormTexBuffer->BindBuffer( GL_RGB32F, vplNormbuffer );
    
}
