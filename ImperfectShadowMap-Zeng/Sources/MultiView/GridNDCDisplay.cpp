#include "GridNDCDisplay.h"
#include "../PointCloudReader/PointCloudReader.h"
#include "../GlobalData/GlobalData.h"

using namespace iglu;

extern SGlobalData* g_data;


void GridNDCDisplay::Initialize( int trackNum )
{

    m_vplPointCloud = new CISMPointCloud(g_data->cornellFileName, 8*8 , 4000, OBJ_CENTER | OBJ_UNITIZE);
    //We use two shaders--one to create the shadow map, another to query the shadow map
    //These are accessed using the enum below
    m_shaderProgram = 0;

    // Location of the light and the eye
    m_eyePos   = vec3(0.0,0.0,2.0);

    // Matrices for setting up the view from the eye
    m_eyeView = IGLUMatrix4x4::LookAt( m_eyePos, vec3::Zero(), vec3::YAxis() );
    m_eyeProj = IGLUMatrix4x4::Perspective( 60, 1, 0.1, 5.0 );

    // Matrices for positioning our geometry relative to the world origin
    m_waveObjModel   = IGLUMatrix4x4::Scale( 0.75f ) * IGLUMatrix4x4::Rotate( -45, vec3::YAxis() );
    m_vertexArrayModel = IGLUMatrix4x4::Translate( -0.45*vec3::YAxis() ) * IGLUMatrix4x4::Scale( 2.0f ) *
        IGLUMatrix4x4::Rotate( 90, vec3::XAxis() );

    // Load a shader that queries a shadow map
    m_shaderProgram = new IGLUShaderProgram("../Shaders/GridView/gridview.vert.glsl", "../Shaders/GridView/gridview.frag.glsl");
    m_shaderProgram->SetProgramEnables(IGLU_GLSL_DEPTH_TEST);

}

void GridNDCDisplay::Display()
{

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    // proj, view change due to trackball
    m_shaderProgram["view"]          = m_eyeView * g_data->trackBall->GetMatrix();
    m_shaderProgram["proj"]      = m_eyeProj;

    // Draw our objects in the correct position
    m_shaderProgram->Enable(); 
    m_shaderProgram["model"] = m_waveObjModel;
    m_shaderProgram["color"] = vec4(.6f, .5f, .8f, 1.0f);
    //waveObj->Draw( shaderProgram ); 
    //g_pointCloud->Draw(shaderProgram);
    m_shaderProgram["model"] = m_waveObjModel;
    m_shaderProgram["color"] = vec4(1.0, .50f, .80f, 1.0f);
    m_shaderProgram["N_set"] = 4;

    m_vplPointCloud->Draw(m_shaderProgram);

    m_shaderProgram->Disable(); 


}
