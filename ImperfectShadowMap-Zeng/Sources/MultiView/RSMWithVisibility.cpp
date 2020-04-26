#include "RSMWithVisibility.h"
#include "../PointCloudReader/ISMPointCloud.h"
#include "../PointCloudReader/PointCloudReader.h"
#include "../Sampler/Sampler.h"
#include "../GlobalData/GlobalData.h"

using namespace iglu;

extern SGlobalData* g_data;

void RSMWithVisibility::Initialize( int trackNum )
{
    g_data->myWin->AddWidget(trackNum,&g_data->rsmDepthBias);
    g_data->myWin->AddWidget(trackNum,&g_data->ismDepthBias);
    g_data->myWin->AddWidget(trackNum,&g_data->rsmIndirectWeight);
    g_data->myWin->AddWidget(trackNum,&g_data->rsmDirectWeight);
    g_data->myWin->AddWidget(trackNum,&g_data->pullPushLevel);
    g_data->myWin->AddWidget(trackNum,&g_data->pullPushInterval);
    g_data->myWin->AddWidget(trackNum,&g_data->rsmLightIntensity);
    g_data->myWin->AddWidget(trackNum,&g_data->rsmLightConeAngle);
    g_data->myWin->AddWidget(trackNum,&g_data->N_SET);
    g_data->myWin->AddWidget(trackNum,&g_data->rsmVisTest);
    g_data->myWin->AddWidget(trackNum,&g_data->ismDrawISM);
    g_data->myWin->AddWidget(trackNum,&g_data->showVPLs);
    g_data->myWin->AddWidget(trackNum,&g_data->sampleFlag);
}

void RSMWithVisibility::Display()
{
    //TODO: clean these
    //glCullFace(	GL_BACK);
    //glEnable(GL_CULL_FACE);

    // Update the light view
    g_data->lightView = g_data->lightCamera->GetMatrix();
    g_data->lightIntensity = g_data->rsmLightIntensity;
    g_data->lightMaxAngle = g_data->rsmLightConeAngle;
    g_data->lightProj = IGLUMatrix4x4::Perspective(g_data->lightMaxAngle, 1.0, g_data->lightNear, g_data->lightFar);
    g_data->N_SET;

    // Generate the reflective shadow map, store result in global data
    GenerateRSM();

    // Make sure you can use textureLod in shader!
    g_data->rsmFbo[g_data->RSM_WORLD_POS].GenerateMipmaps(false);
    g_data->rsmFbo[g_data->RSM_WORLD_NORMAL].GenerateMipmaps(false);
    g_data->rsmFbo[g_data->RSM_FLUX].GenerateMipmaps(false);
    g_data->rsmFbo[g_data->RSM_WORLD_POS].SetTextureParameters(IGLU_MIN_LINEAR_MIP_LINEAR);
    g_data->rsmFbo[g_data->RSM_WORLD_NORMAL].SetTextureParameters(IGLU_MIN_LINEAR_MIP_LINEAR);
    g_data->rsmFbo[g_data->RSM_FLUX].SetTextureParameters(IGLU_MIN_LINEAR_MIP_LINEAR);

    // Now we use Chris's VSV demo's method to sample texture to VPL
    // No rasterization
    SampleVPLs(g_data->rsmFbo, false);

    // Generate multiple VPLs' ISM
    GenerateISM(g_data->vplTexBuffer);

    // Pull and push to fix holes
    PullPushPhase(g_data->ismGenFbo[IGLU_DEPTH], g_data->pullPushLevel, g_data->pullPushInterval);

    // Render using RSM and ISM
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    ForwardQueryRSMwISM(g_data->vplTexBuffer, *(g_data->pushTexArray[0]), g_data->rsmFbo);

    //Draw the light
    DrawMainLight();

    // Draw the VPL
    if (g_data->showVPLs)
    {
        DrawVPLs(g_data->vplTexBuffer, g_data->arrowObj);
    }

    if (g_data->ismDrawISM)
    {
        IGLUDraw::Fullscreen(g_data->pushTexArray[0]);
    }

}

RSMWithVisibility::RSMWithVisibility() : IGLUDisplayMode()
{

}
