#include "SimplePointCloudViewer.h"
#include "../PointCloudReader/PointCloudReader.h"
#include "../GlobalData/GlobalData.h"

using namespace iglu;

extern SGlobalData* g_data;


void SimplePointCloudViewer::Initialize( int trackNum )
{
}

void SimplePointCloudViewer::Display()
{

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    // proj, view change due to trackball
    g_data->simpleShader["view"]          = g_data->camera->GetMatrix();
    g_data->simpleShader["proj"]      = g_data->proj;

    // Draw our objects in the correct position
    g_data->simpleShader->Enable(); 
    g_data->simpleShader["model"] = IGLUMatrix4x4::Identity();
    g_data->simpleShader["color"] = vec4(.6f, .5f, .8f, 1.0f);
    g_data->cornellPtCloud->Draw(g_data->simpleShader);

    g_data->simpleShader->Disable(); 


}

SimplePointCloudViewer::SimplePointCloudViewer() : IGLUDisplayMode()
{

}
