#include "ISMPointCloud.h"
#include "PointCloudReader.h"
using namespace iglu;


struct PointCloud 
{
    float pos[3];
    int id;
};

CISMPointCloud::CISMPointCloud(char* fileName, int vplNumber, int pointNumber, bool unitizeAndResize)
    :vertexGeom(0), m_maxVplNumber(vplNumber), m_drawVplNumber(vplNumber)
{
    m_originalPointCloud = new CPointCloudReader(fileName, pointNumber, (unitizeAndResize? (OBJ_CENTER | OBJ_UNITIZE) : DEFAULT_STATE) );

    //Create the point cloud using CPoint cloud function
    PointCloud* pointCloud4x4 = new PointCloud[m_originalPointCloud->GetRawFloatSize()*vplNumber/3];

    int listSize = m_originalPointCloud->GetRawFloatSize()/3;
    float* rawData = m_originalPointCloud->GetRawFloatData();
    for (int i = 0; i < vplNumber; ++i)
    {
        for (int j = 0; j < listSize; ++j)
        {
            pointCloud4x4[j+i*listSize].pos[0] = rawData[j*3+0];
            pointCloud4x4[j+i*listSize].pos[1] = rawData[j*3+1];
            pointCloud4x4[j+i*listSize].pos[2] = rawData[j*3+2];
            pointCloud4x4[j+i*listSize].id = i;
        }
    }

    //Set the float for vertex
    vertexGeom = new IGLUVertexArray();
    vertexGeom->SetVertexArray( sizeof( PointCloud )*listSize*vplNumber, pointCloud4x4, IGLU_STATIC|IGLU_DRAW );
    vertexGeom->EnableAttribute( IGLU_ATTRIB_VERTEX, 3, GL_FLOAT, sizeof( PointCloud ), BUFFER_OFFSET(0) );
    vertexGeom->EnableAttribute( IGLU_ATTRIB_MATL_ID, 1, GL_FLOAT, sizeof( PointCloud ), BUFFER_OFFSET(3*sizeof(float)) );

}

CISMPointCloud::CISMPointCloud( CPointCloudReader& pointCloud, int vplNumber )
{
    //Construct from a pointcloudreader object
}

CISMPointCloud::~CISMPointCloud()
{

}

int CISMPointCloud::Draw( iglu::IGLUShaderProgram::Ptr &shader )
{
    //if (shader == 0)
    //{
    //    //Create default shader
    //}
    //simply draw the geometry
    vertexGeom->DrawArrays( GL_POINTS, 0, m_originalPointCloud->GetRawFloatSize()/3 *m_drawVplNumber );
    return true;
}

int CISMPointCloud::Draw( iglu::IGLUShaderProgram::Ptr &shader, unsigned int instanceNumber )
{
    vertexGeom->DrawArrays( GL_POINTS, 0, m_originalPointCloud->GetRawFloatSize()/3 *instanceNumber );
    m_drawVplNumber = instanceNumber;
    return true;
}
