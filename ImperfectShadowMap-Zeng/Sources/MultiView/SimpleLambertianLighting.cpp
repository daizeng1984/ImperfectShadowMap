#include "SimpleLambertianLighting.h"
#include "../PointCloudReader/PointCloudReader.h"
#include "../GlobalData/GlobalData.h"

using namespace iglu;

extern SGlobalData* g_data;


void SimpleLambertianLighting::Initialize( int trackNum )
{
}

void SimpleLambertianLighting::Display()
{

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    // proj, view, model change due to trackball
    g_data->lambertShader["view"]          = g_data->camera->GetMatrix();
    g_data->lambertShader["proj"]      = g_data->proj;
    g_data->lambertShader["model"] = IGLUMatrix4x4::Identity();
    g_data->lambertShader["lightPos"] = g_data->lightCamera->GetPosition();
    g_data->lambertShader["matlInfoTex"] = IGLUOBJMaterialReader::s_matlCoefBuf;

    g_data->cornellObj->Draw(g_data->lambertShader);

}

SimpleLambertianLighting::SimpleLambertianLighting() : IGLUDisplayMode()
{

}
