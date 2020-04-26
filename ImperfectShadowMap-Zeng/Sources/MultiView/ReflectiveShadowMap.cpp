#include "ReflectiveShadowMap.h"
#include "../PointCloudReader/PointCloudReader.h"
#include "../GlobalData/GlobalData.h"

using namespace iglu;

extern SGlobalData* g_data;

uint GetDisplayType(uint displayType)
{
    if (displayType < 3)
    {
        return displayType;
    }
    else
    {
        return IGLU_DEPTH;
    }
}
void ReflectiveShadowMap::Initialize( int trackNum )
{
    g_data->myWin->AddWidget(trackNum,&g_data->rsmLightIntensity);
    g_data->myWin->AddWidget(trackNum,&g_data->rsmLightConeAngle);
    g_data->myWin->AddWidget(trackNum,&g_data->N_SET);
    g_data->myWin->AddWidget(trackNum,&g_data->sampleFlag );
}

void ReflectiveShadowMap::Display()
{

    // proj, view change due to trackball, camera
    g_data->lightView = g_data->lightCamera->GetMatrix();

    // Generate the reflective shadow map, store result in global data
    GenerateRSM();
    // Make sure you can use textureLod in shader!
    g_data->rsmFbo[g_data->RSM_WORLD_POS].GenerateMipmaps(false);
    g_data->rsmFbo[g_data->RSM_WORLD_NORMAL].GenerateMipmaps(false);
    g_data->rsmFbo[g_data->RSM_FLUX].GenerateMipmaps(false);
    g_data->rsmFbo[g_data->RSM_WORLD_POS].SetTextureParameters(IGLU_MIN_LINEAR_MIP_LINEAR);
    g_data->rsmFbo[g_data->RSM_WORLD_NORMAL].SetTextureParameters(IGLU_MIN_LINEAR_MIP_LINEAR);
    g_data->rsmFbo[g_data->RSM_FLUX].SetTextureParameters(IGLU_MIN_LINEAR_MIP_LINEAR);


    // Clear framebuffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    //Let's see the MRTs
    IGLUDraw::Fullscreen(g_data->rsmFbo[GetDisplayType(g_data->currentRsmType)]);
    // Now we use Chris's VSV demo's method to sample texture to VPL
    // No rasterization
    glPointSize(6.0f);
    SampleVPLs(g_data->rsmFbo, true);
    glPointSize(1.0f);
}

ReflectiveShadowMap::ReflectiveShadowMap() : IGLUDisplayMode()
{


}
