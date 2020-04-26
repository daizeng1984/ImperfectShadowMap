#ifndef __ISMPointCloud_h__
#define __ISMPointCloud_h__

// This class use CPointCloud to load obj to point cloud and copy this 
//    point cloud based on the VPL number or grid number, I might change
//    it to other methods like instancing or directly draw multiple time
//    and test their performance.

#include "PointCloudReader.h"

class CISMPointCloud
{
public:
    CISMPointCloud(char* fileName, int vplNumber, int pointNumber, bool unitizeAndResize = false);
    CISMPointCloud(CPointCloudReader& pointCloud, int vplNumber);
    ~CISMPointCloud();

    //Draw
    int Draw( iglu::IGLUShaderProgram::Ptr &shader );
    int Draw( iglu::IGLUShaderProgram::Ptr &shader, unsigned int instanceNumber);

    //Set VPL number for drawing
    void SetVPLNumber(unsigned int vplNumber) {m_drawVplNumber = vplNumber;}

private:
    //Point cloud original copy
    CPointCloudReader*  m_originalPointCloud;

    //Gpu buffer
    iglu::IGLUVertexArray::Ptr vertexGeom;
    
    //vertex size
    int m_maxVplNumber;

    //actually drawing VPL number
    int m_drawVplNumber;
};

#endif // __ISMPointCloud_h__
