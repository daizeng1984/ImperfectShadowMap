#include "GlobalData.h"
#include "../PointCloudReader/PointCloudReader.h"

extern SGlobalData *g_data;

const IGLUMatrix4x4 & GetDefaultViewMatrix()
{
    // Matrices for setting up the view
    static const IGLUMatrix4x4 view = IGLUMatrix4x4::LookAt(vec3(2.78, 2.73, -8.00),vec3(2.78, 2.73, 2.795), vec3::YAxis() );
    return view;
}

void OpenGLInitialization( void )
{
    //Create Cornell box's obj and point cloud
    g_data->cornellFileName = strdup("../../CommonSampleFiles-Zeng/Models/myCornellBox.obj");
    g_data->cornellObj = new IGLUOBJReader(g_data->cornellFileName, OBJ_CENTER | OBJ_UNITIZE );
    g_data->cornellPtCloud = new CPointCloudReader(g_data->cornellFileName, g_data->POINT_NUMBER_EACH_INSTANCE, OBJ_CENTER | OBJ_UNITIZE);
    //g_data->cornellMultipleVplPCloud = new CISMPointCloud(g_data->cornellFileName, g_data->MAX_N_SET*g_data->MAX_N_SET , g_data->POINT_NUMBER_EACH_INSTANCE, true );
    //Create a very simple sphere object 
    g_data->sphereFileName = strdup("../../CommonSampleFiles-Zeng/Models/sphere_rough.obj");
    g_data->sphereObj = new IGLUOBJReader(g_data->sphereFileName, OBJ_CENTER | OBJ_UNITIZE );
    //Create a very simple arraw object used to represent light
    g_data->arrowFileName = strdup("../../CommonSampleFiles-Zeng/Models/arrow.obj");
    g_data->arrowObj = new IGLUOBJReader(g_data->arrowFileName, OBJ_CENTER | OBJ_UNITIZE );
    //Finalilze the materials
    IGLUOBJMaterialReader::FinalizeMaterialsForRendering();

    // Create a virtual trackball
    g_data->trackBall = new IGLUTrackball( g_data->myWin->w(), g_data->myWin->h() );
    g_data->camera = new CMyCamera(g_data->myWin->w(), g_data->myWin->h());
    g_data->camera->SetView(vec3(0, 0, -4.00), vec3::Zero(), vec3::YAxis());
    g_data->camera->SetSpeed(0.05);

    // Create simple view shader
    g_data->simpleShader = new IGLUShaderProgram("../Shaders/SimpleView/simpleview.vert.glsl"
                                               , "../Shaders/SimpleView/simpleview.frag.glsl");
    // Create lambertian shader
    g_data->lambertShader = new IGLUShaderProgram("../Shaders/SimpleView/simplelambert.vert.glsl"
                                               , "../Shaders/SimpleView/simplelambert.frag.glsl");
    // Create ISM generation shader
    g_data->ismGenShader = new IGLUShaderProgram("../Shaders/ISMGeneration/ISMGeneration.vert.glsl"
                                                ,"../Shaders/ISMGeneration/ISMGeneration.geom.glsl"
                                                , "../Shaders/ISMGeneration/ISMGeneration.frag.glsl");

    g_data->ismGenFbo = IGLUFramebuffer::Create(GL_RGBA8, g_data->SHADOWMAP_SIZE , g_data->SHADOWMAP_SIZE, true, false);

    // Create RSM
    g_data->rsmShader = new IGLUShaderProgram("../Shaders/RSM/RSMGen3mrt.vert.glsl"
                                            , "../Shaders/RSM/RSMGen3mrt.frag.glsl");
    g_data->rsmWithVisQueryShader = new IGLUShaderProgram("../Shaders/RSMWithVis/RSMWithVis.vert.glsl"
                                                            , "../Shaders/RSMWithVis/RSMWithVis.frag.glsl");
    //It's important to add depth buffer by setting the 4th parameter to true!
    g_data->rsmFbo = IGLUFramebuffer::CreateMRT(GL_RGBA32F, g_data->RSM_SIZE, g_data->RSM_SIZE, 3, true);
    g_data->lightNear = 0.1;
    g_data->lightFar = 30.0;
    g_data->lightMaxAngle = 60.0;
    g_data->lightIntensity = 2.0;
    g_data->lightView = GetDefaultViewMatrix();
    g_data->lightProj = IGLUMatrix4x4::Perspective(g_data->lightMaxAngle, 1.0, g_data->lightNear, g_data->lightFar);

    // Other data
    g_data->currentRsmType = g_data->RSM_FLUX;
    //g_data->N_SET = g_data->MAX_N_SET;

    // Create Pull and push hierarchies
    for (int i = 0; i < g_data->SHADOWMAP_MIP_MAP_LEVEL; ++ i)
    {
        g_data->pullTexArray[i] = new IGLURenderTexture2D( g_data->SHADOWMAP_SIZE/int(powf(2,i))
            , g_data->SHADOWMAP_SIZE/int(powf(2,i)), GL_RGB32F ); //32 bit depth info only, but red is not good to see the depth map

        g_data->pPullSamplerArray[i] = new CSampler("../Shaders/ISMGeneration/pull.frag.glsl", g_data->pullTexArray[i]);

        g_data->pushTexArray[i] = new IGLURenderTexture2D( g_data->SHADOWMAP_SIZE/int(powf(2,i))
            , g_data->SHADOWMAP_SIZE/int(powf(2,i)), GL_RGB32F ); //32 bit depth info only

        g_data->pPushSamplerArray[i] = new CSampler("../Shaders/ISMGeneration/push.frag.glsl", g_data->pushTexArray[i]);
    }

    // Matrix initialize
    g_data->proj = IGLUMatrix4x4::Perspective(40, 1.0, g_data->lightNear, g_data->lightFar);

    // UI parameter intialization
    g_data->rsmDepthBias.Initialize( -0.0002, IGLURange<float>(-0.001,0), 0.00001, "RSM depth bias");
    g_data->ismDepthBias.Initialize( -0.01, IGLURange<float>(-0.01,0), 0.0002, "ISM depth bias");
    g_data->rsmIndirectWeight.Initialize( 2.0, IGLURange<float>(0,4), 0.001, "Indirect light intensity");
    g_data->rsmDirectWeight.Initialize( 1.0, IGLURange<float>(0,2), 0.001, "Direct light intensity");
    g_data->pullPushLevel.Initialize( 3, IGLURange<int>(1,g_data->SHADOWMAP_MIP_MAP_LEVEL), 1, "Pull Push Level");
    g_data->pullPushInterval.Initialize( 0.0001, IGLURange<float>(0,0.1), 0.000001, "Pull Push depth threshold");
    g_data->rsmLightIntensity.Initialize( 2, IGLURange<float>(0,8), 0.1, "Spot light intensity");
    g_data->rsmLightConeAngle.Initialize( 60, IGLURange<float>(0,180), 5, "Spot light cone angle");
    g_data->N_SET.Initialize( 8, IGLURange<int>(1,32), 1, "VPL number: n x n");
    g_data->rsmVisTest.Initialize( true, "Test visibility?");
    g_data->ismDrawISM.Initialize( false, "Draw ISM texture?");
    g_data->showVPLs.Initialize( false, "Draw VPLs?");
    g_data->sampleFlag.Initialize( 0, IGLURange<int>(0,3), 1, "Sampling Type");

    // Light Camera
    g_data->lightCamera = new CMyCamera(g_data->myWin->w(), g_data->myWin->h());
    g_data->lightCamera->SetView(vec3(0.8, 0.8, -0.8), vec3::Zero(), vec3::YAxis());
    g_data->lightCamera->SetSpeed(0.01);
    g_data->lightCamera->SetKeys("ikjl;/");

    // Garbage point for sampling taken from Chris' VSV Demo
    // Create a dummy vertex array used for our rectification pass.  This is really sort
    //    of silly -- filling up a garbage vertex array so the GPU can ignore it. But it's
    //    easy and straightforward.  The garbage array has 2 floats for MAX_N_SET*MAX_N_SET
    //    points
    int numGarbagePoints = 2 * g_data->MAX_N_SET * g_data->MAX_N_SET;
    float *garbagePtr = (float *)malloc( sizeof(float) * numGarbagePoints );
    GridSamplePointBuffer(garbagePtr, numGarbagePoints );
    g_data->garbagePoints = new IGLUVertexArray();
    g_data->garbagePoints->SetVertexArray( numGarbagePoints * sizeof(float), garbagePtr, IGLU_DYNAMIC|IGLU_STREAM );
    g_data->garbagePoints->EnableAttribute( IGLU_ATTRIB_VERTEX, 2, GL_FLOAT );

    // Create a buffer (used as input storage for a VPL buffer texture, and
    //    as output from transform feedback sampling of video into VPLs)
    g_data->vplRawBuffer = new IGLUBuffer();
    g_data->vplRawBuffer->SetBufferData( 3*4*sizeof(float)*g_data->MAX_N_SET * g_data->MAX_N_SET );  // Up to g_data->MAX_N_SET * g_data->MAX_N_SET VPLs

    // Our texture used to store individual VPLs.
    g_data->vplTexBuffer = new IGLUTextureBuffer();
    g_data->vplTexBuffer->BindBuffer( GL_RGBA32F, g_data->vplRawBuffer );

    // Create the transform feedback for vpl buffer
    // Create an OpenGL transform feedback object and bind it to our buffer.
    g_data->vplTransformfeeback = new IGLUTransformFeedback();
    g_data->vplTransformfeeback->AttachBuffer( g_data->vplRawBuffer );

    // Load a shader that sample vpl from RSM texture
    // Initialize shader that performs the sampling via transform feedback
    const char *xformFeedbackNames[3] = { "gl_Position","vplNormal", "vplFlux"  };
    g_data->vplSamplingShader = new IGLUShaderProgram( "../Shaders/RSMWithVis/VPLSampling.vert.glsl", 
                                                        "../Shaders/RSMWithVis/VPLSampling.geom.glsl",
                                                        "../Shaders/RSMWithVis/VPLSampling.frag.glsl" );
    g_data->vplSamplingShader->SetTransformFeedbackVaryings( 3, xformFeedbackNames );

    // VPL drawing shader
    g_data->vplDrawingShader = new IGLUShaderProgram( "../Shaders/RSMWithVis/DrawVPLs.vert.glsl", 
                                                     "../Shaders/RSMWithVis/DrawVPLs.frag.glsl" );

    // Normal random texture
    g_data->randTex2D = new IGLURandomTexture2D( g_data->MAX_N_SET, g_data->MAX_N_SET, GL_FLOAT );
}

void Motion( int x, int y )
{
    g_data->trackBall->UpdateOnMotion(x, y);
    g_data->camera->UpdateOnMotion(x, y);
    g_data->lightCamera->UpdateOnMotion(x, y);
}

void Button( int button, int state, int x, int y )
{
    if(IGLU_EVENT_DOWN == state)
    {
        if (button == IGLU_EVENT_LEFT_BUTTON)
        {
            g_data->trackBall->SetOnClick( x, y );
        }
        else if (button == IGLU_EVENT_RIGHT_BUTTON)
        {
            g_data->camera->SetOnClick(x, y);
        }
        else if (button ==  IGLU_EVENT_MIDDLE_BUTTON)
        {
            g_data->lightCamera->SetOnClick(x, y);
        }
    }
    else
    {
        if (button == IGLU_EVENT_LEFT_BUTTON)
        {
            g_data->trackBall->Release();
        }
        else if (button == IGLU_EVENT_RIGHT_BUTTON)
        {
            g_data->camera->Release();
        }
        else if (button ==  IGLU_EVENT_MIDDLE_BUTTON)
        {
            g_data->lightCamera->Release();
        }
    }
}

void Keys( unsigned char key, int x, int y )
{
    g_data->camera->UpdateOnKeys( key, x, y);
    g_data->lightCamera->UpdateOnKeys( key, x, y);
    if (key == '=' || key == '+')
    {
        g_data->currentRsmType = (g_data->currentRsmType + 1)%g_data->RSM_TYPE_NUM;
    }
}

void GenerateRSM()
{
    // Render to MRT
    g_data->rsmFbo->Bind();
    g_data->rsmFbo->Clear();
    {
        g_data->rsmShader->Enable();
        g_data->rsmShader["view"]          = g_data->lightView;
        g_data->rsmShader["proj"]      = g_data->lightProj;
        g_data->rsmShader["model"] = IGLUMatrix4x4::Identity();//g_data->trackBall->GetMatrix();
        //Other parameters
        g_data->rsmShader["maxConeAngle"] = g_data->lightMaxAngle/2.0;
        g_data->rsmShader["matlInfoTex"] = IGLUOBJMaterialReader::s_matlCoefBuf;
        g_data->rsmShader["lightIntensity"] = g_data->lightIntensity;
        g_data->cornellObj->Draw(g_data->rsmShader);

        g_data->rsmShader->Disable();
    }
    g_data->rsmFbo->Unbind();
}

void GenerateISM( iglu::IGLUTextureBuffer::Ptr vplPosBuffer )
{
    // Generate multiple VPLs' ISM
    g_data->ismGenFbo->Bind();
    g_data->ismGenFbo->Clear();
    {
        g_data->ismGenShader->Enable();
        // Set the program texture uniform
        g_data->ismGenShader["vplBuffer"] = vplPosBuffer;
        g_data->ismGenShader["nearDist"] = g_data->lightNear;
        g_data->ismGenShader["farDist"] = g_data->lightFar;
        g_data->ismGenShader["N_set"] = g_data->N_SET;
        g_data->ismGenShader["model"] = IGLUMatrix4x4::Identity();
        // Draw the scene
        g_data->cornellPtCloud->Draw(g_data->ismGenShader, g_data->N_SET*g_data->N_SET);
        g_data->ismGenShader->Disable(); 
    }
    g_data->ismGenFbo->Unbind();
}

void PullPushPhase( iglu::IGLURenderTexture& srcTex, int maxPullPushLevel, float depthInterval )
{
    assert(maxPullPushLevel < g_data->SHADOWMAP_MIP_MAP_LEVEL);
    //Pull and Push and save the scr texture to pushTexArray[0] as the level 0 for pull/push hierarchies
    g_data->pPullSamplerArray[0]->CopyFrom(&srcTex);
    for (int i = 0; i < maxPullPushLevel -1 ; ++ i)
    {
        g_data->pPullSamplerArray[i+1]["depthInterval"] = depthInterval;
        g_data->pPullSamplerArray[i+1]->SampleFrom(g_data->pullTexArray[i]);
    }

    //Copy the coarsest texture from pull hierarchies to push hierarchies so that we could start the push process
    g_data->pPushSamplerArray[maxPullPushLevel-1]->CopyFrom(g_data->pullTexArray[maxPullPushLevel-1]);
    for (int i = maxPullPushLevel -1; i > 0  ; -- i)
    {
        g_data->pPushSamplerArray[i-1]["finerTex"] = g_data->pullTexArray[i-1];
        g_data->pPushSamplerArray[i-1]["depthInterval"] = depthInterval;
        g_data->pPushSamplerArray[i-1]->SampleFrom(g_data->pushTexArray[i]);
    }

    //So after the process pushTexArray[0] will be the result if maxPullPushLevel > 2
    if (maxPullPushLevel < 2)
    {
        g_data->pPushSamplerArray[0]->SampleFrom(&srcTex, false);
    }
}

void DrawMainLight()
{
    // Get the inverse of light camera view
    IGLUMatrix4x4 inverseLightView = g_data->lightCamera->GetMatrix();
    inverseLightView = inverseLightView.Invert();
    // Set all necessary parameter to shader
    g_data->simpleShader["view"] = g_data->camera->GetMatrix();
    g_data->simpleShader["proj"] = g_data->proj;
    g_data->simpleShader["model"] = inverseLightView*IGLUMatrix4x4::Scale(0.1);
    g_data->simpleShader["color"] = vec4(1, 1, 0, 1);
    // Draw the light mesh, which is an arrow
    g_data->arrowObj->Draw(g_data->simpleShader);
}

void ForwardQueryRSMwISM( iglu::IGLUTextureBuffer::Ptr vplPosBuffer, iglu::IGLUTexture& ism, iglu::IGLUFramebuffer::Ptr rsmFBo )
{
    g_data->rsmWithVisQueryShader->Enable();
    {
        // Matrix
        g_data->rsmWithVisQueryShader["lightView"] = g_data->lightView;
        g_data->rsmWithVisQueryShader["lightProj"] = g_data->lightProj;
        g_data->rsmWithVisQueryShader["model"] = IGLUMatrix4x4::Identity();
        g_data->rsmWithVisQueryShader["view"] = g_data->camera->GetMatrix();
        g_data->rsmWithVisQueryShader["proj"] = g_data->proj;

        // Textures
        g_data->rsmWithVisQueryShader["rsmDepth"] = rsmFBo[IGLU_DEPTH]; // Depth
        g_data->rsmWithVisQueryShader["ism"] = ism;//g_data->ismGenFbo[IGLU_DEPTH];
        g_data->rsmWithVisQueryShader["vplBuffer"] = vplPosBuffer;

        // Material
        g_data->rsmWithVisQueryShader["matlInfoTex"] = IGLUOBJMaterialReader::s_matlCoefBuf;

        // Other parameters
        g_data->rsmWithVisQueryShader["nearDist"] = g_data->lightNear;
        g_data->rsmWithVisQueryShader["farDist"] = g_data->lightFar;
        g_data->rsmWithVisQueryShader["N_set"] = g_data->N_SET;
        g_data->rsmWithVisQueryShader["maxConeAngle"] = g_data->lightMaxAngle/2.0;
        g_data->rsmWithVisQueryShader["lightIntensity"] = g_data->lightIntensity;
        g_data->rsmWithVisQueryShader["rsmDepthBias"] = g_data->rsmDepthBias;
        g_data->rsmWithVisQueryShader["ismDepthBias"] = g_data->ismDepthBias;
        g_data->rsmWithVisQueryShader["rsmIndirectWeight"] = g_data->rsmIndirectWeight;
        g_data->rsmWithVisQueryShader["rsmDirectWeight"] = g_data->rsmDirectWeight;
        g_data->rsmWithVisQueryShader["noVisTest"] = (!(g_data->rsmVisTest));

        // Draw the triangle mesh of scene
        g_data->cornellObj->Draw(g_data->rsmWithVisQueryShader);
    }
    g_data->rsmWithVisQueryShader->Disable();

}

void DrawVPLs( iglu::IGLUTextureBuffer::Ptr vplBuffer, iglu::IGLUOBJReader::Ptr lightObj )
{
    IGLUVertexArray::Ptr vplObjArray = lightObj->GetVertexArray();

    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    g_data->vplDrawingShader->Enable();
    g_data->vplDrawingShader["vplBuffer"] = vplBuffer;
    g_data->vplDrawingShader["view"] = g_data->camera->GetMatrix();
    g_data->vplDrawingShader["proj"] = g_data->proj;

    // Setup the attributes needed for this geometry
    //vplObjArray->EnableAttribute( IGLU_ATTRIB_VERTEX, 3, GL_FLOAT, g_data->sphereObj->GetArrayBufferStride(), BUFFER_OFFSET(0));

    vplObjArray->DrawArraysInstanced( GL_TRIANGLES, 0, 3* lightObj->GetTriangleCount(), g_data->N_SET*g_data->N_SET);

    g_data->vplDrawingShader->Disable();
    //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

float* Vec2ToFloatBuffer(float* buffer, const vec2& vec)
{
    buffer[0] = vec.X();
    buffer[1] = vec.Y();
    return buffer + 2;
}

// rect[0,1] = is lowerleft x, y; rect[2,3] is upperright
float* _recursive_buffer(float* buffer, vec2 lowerLeft, vec2 upperRight, unsigned int& bufferLength)
{
    if ( bufferLength < 2)
    {
        return buffer;
    }
    //First fill the 3 points: left, down, and center (x)
    //o---o---o
    //|   |   |
    //x---x---o
    //|   |   |
    //o---x---o
    vec2* point[5] = {0};
    vec2 center = 0.5*(lowerLeft + upperRight);
    vec2 left(lowerLeft.X(), center.Y());
    vec2 down(center.X(), lowerLeft.Y());
    vec2 up(center.X(), upperRight.Y());
    vec2 right(upperRight.X(), center.Y());
    point[0] = &center;
    point[1] = &left;
    point[2] = &down;

    
    //Second fill the rest of the 2 points: up, right if they are on the boundary
    const float epsilon = 1e-4;
    if (upperRight.X() >= 1.0 - epsilon)
    {
        point[3] = &right;
    }

    if (upperRight.Y() >= 1.0 - epsilon)
    {
        point[4] = &up;
    }

    // Start to fill them!
    for (int i = 0; i < 5; ++ i)
    {
        if ( bufferLength < 2)
        {
            return buffer;
        }
        if (point[i])
        {
            buffer = Vec2ToFloatBuffer(buffer, *(point[i]));
            bufferLength -= 2;
        }
    }


    return buffer;
}

void GridSamplePointBuffer( float * buffer, unsigned int bufferLength )
{
    vec2 lowerLeft(0, 0);
    vec2 upperRight(1, 1);
    vec2 left(0, 1);
    vec2 right(1, 0);

    buffer = Vec2ToFloatBuffer(buffer, lowerLeft);
    buffer = Vec2ToFloatBuffer(buffer, upperRight);
    buffer = Vec2ToFloatBuffer(buffer, left);
    buffer = Vec2ToFloatBuffer(buffer, right);
    bufferLength -= 2*4;
    buffer = _recursive_buffer(buffer, lowerLeft, upperRight, bufferLength);

    int n = 2;
    while (bufferLength > 0)
    {
        float cellDelta = (1.0/n);
        for (int i = 0; i < n; ++i)
        {
            for(int j = 0; j < n; ++ j)
            {
                vec2 tmpLowerLeft(i*cellDelta, j*cellDelta);
                vec2 tmpUpperRight((i+1)*cellDelta, (j+1)*cellDelta);
                buffer = _recursive_buffer(buffer, tmpLowerLeft, tmpUpperRight, bufferLength);
            }
        }
        n = n*2;
    }
    //_recursive_buffer(buffer, lowerLeft, upperRight, bufferLength);
}

void SampleVPLs( iglu::IGLUFramebuffer::Ptr rsmFbo, bool outputTexSamplePoints /*= false*/ )
{
    if (!outputTexSamplePoints)
    {
        glEnable( GL_RASTERIZER_DISCARD );
    }
    g_data->vplSamplingShader->Enable();
    g_data->vplTransformfeeback->Begin( GL_POINTS );
        g_data->vplSamplingShader["rsmPos"]   =     rsmFbo[g_data->RSM_WORLD_POS];
        g_data->vplSamplingShader["rsmNormal"]    = rsmFbo[g_data->RSM_WORLD_NORMAL];
        g_data->vplSamplingShader["rsmFlux"] =      rsmFbo[g_data->RSM_FLUX];
        g_data->vplSamplingShader["sampleFlag"] =      g_data->sampleFlag;
        g_data->vplSamplingShader["randTex2D"] =      g_data->randTex2D;
        g_data->vplSamplingShader["debugMode"] =      outputTexSamplePoints;
    // Draw the garbage points!
    g_data->garbagePoints->DrawArrays( GL_POINTS, 0, g_data->N_SET* g_data->N_SET);

    g_data->vplTransformfeeback->End();
    g_data->vplSamplingShader->Disable();

    if (!outputTexSamplePoints)
    {
        glDisable( GL_RASTERIZER_DISCARD );
    }
}
