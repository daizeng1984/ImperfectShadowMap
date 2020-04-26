#include "PointCloudReader.h"
using namespace iglu;

struct CPointCloudReader::STriangle
{
    uint vIdx[3];
    float area;
};

CPointCloudReader::CPointCloudReader( const char * fileName, iglu::uint sampleNumber, int flag /*= DEFAULT_STATE*/ ):
IGLUFileParser( (char*)fileName ), IGLUModel(), m_randomGenerator(), m_recenter(false), m_resize(false), m_sampleNumber(sampleNumber),m_totalArea(0), m_vertArrPointCloud(0)
{
    //Configure the flag
    m_resize        = flag & OBJ_UNITIZE ? true : false;
    m_recenter        = flag & OBJ_CENTER ? true : false;

    //Load the file using parser(already done in constructor of IGLUFileParser
    //Then parse the file and store the raw information
    ParseRawTriangle();

    //Recenter and resize
    AdjustCenterAndSize(m_objVerts);

    //Compute the area for each triangle
    ComputeTriangleArea();

    //Generate point cloud
    RegeneratePointCloud();

    //Create GPU vertex buffer
    CreateGpuVertexArray();
}

CPointCloudReader::~CPointCloudReader()
{
    delete m_vertArrPointCloud;
    //Delete the triangle information list
    for (int i = 0; i < m_objTri.Size(); ++i)
    {
        if(m_objTri[i])
        {
            delete m_objTri[i];
        }
    }
}

void CPointCloudReader::RegeneratePointCloud()
{
    float samplePerArea = float(m_sampleNumber)/m_totalArea;
    // Create the point list
    IGLUArray1D<vec3*> tempPointList;
    tempPointList.SetSize(0);
    // for each triangle sample proportional to its area at least once for each triangle
    for (int i = 0; i < m_objTri.Size(); ++i)
    {
        //get each triangle's 3 vertices
        vec3 v[3];
        for (int j = 0; j < 3; ++j)
        {
            v[j] = m_objVerts[m_objTri[i]->vIdx[j]];
        }

        //Sample number based on average sample, we possibly could round the float number using floor(0.5+x)
        for (int j = 0; j < ceil(samplePerArea*m_objTri[i]->area); ++j)
        {
            vec3 *p = new vec3(RandomTriangle(v));
            tempPointList.Add(p);
        }
    }

    uint newSampleNumber = tempPointList.Size();
    assert(newSampleNumber >= m_sampleNumber);

    //Actually we generate more sample than we expected, so we remove the sample uniformly
    //Take the extra sample out by delete its triangle information
    for (int i = 0; i < newSampleNumber - m_sampleNumber; )
    {
        int rIdx = m_randomGenerator.sRandom()%newSampleNumber;
        if (tempPointList[rIdx])
        {
            delete tempPointList[rIdx];
            tempPointList[rIdx] = 0;
            ++i;
        }
    }
    
    //Move all the point list to finalize our member
    //m_pointList.Clear();
    //TODO: new replacement

    for (int i = 0; i < tempPointList.Size(); ++i )
    {
        if (tempPointList[i])
        {
            //Store the point list to member
            m_pointList.Add(tempPointList[i]->X());
            m_pointList.Add(tempPointList[i]->Y());
            m_pointList.Add(tempPointList[i]->Z());
            //Remove the point
            delete tempPointList[i];
            tempPointList[i] = 0;
        }
    }
}

void CPointCloudReader::Save( const char * fileName )
{
    //TODO:
    //Write header

    //Write raw float data
}

bool CPointCloudReader::IsValidFLine()
{
    //Garbage variables. This will break if it comes accross an entry on a line
    //longer than 128 characters. 
    char c0[128];
    char c1[128];
    char c2[128];
    char vertToken[128]; 

    // Non-destructively parse the next token
    char *lPtr = this->GetCurrentLinePointer();
    StripLeadingTokenToBuffer( lPtr, vertToken );

    //Make sure we have at least three entries in this vertex
    if(sscanf(lPtr, "%s %s %s", c0, c1, c2) >= 3){
        return true;
    }else{
        this->WarningMessage("Corrupt 'f': %s", lPtr);
        return false; 
    }
}

int CPointCloudReader::Draw( IGLUShaderProgram::Ptr &shader, unsigned int instanceNumber, bool usePointCloud )
{
    m_vertArrPointCloud->DrawArraysInstanced(GL_POINTS, 0, m_pointList.Size()/3, instanceNumber);
    return 1;
}

int CPointCloudReader::Draw( iglu::IGLUShaderProgram::Ptr &shader )
{
    m_vertArrPointCloud->DrawArrays(GL_POINTS, 0, m_pointList.Size()/3);
    return 1;

}

void CPointCloudReader::ParseRawTriangle()
{
    char *linePtr = 0;

    char keyword[64];
    STriangle* curTri;

    while ( (linePtr = this->ReadNextLine()) != NULL )
    {
        // Each OBJ line starts with a keyword/keyletter
        this->GetLowerCaseToken( keyword );

        switch( keyword[0] )
        {
        case 'v': 
            if (keyword[1] == 0 )    // We only need a vertex!
                m_objVerts.Add( this->GetVec3() );  
            break;
        case 'm': 
            //We simple ignore since we only generate point cloud in this case
            break;
        case 'o': 
            //We simple ignore since we only generate point cloud in this case
            break;
        case 'g': 
            //We simple ignore since we only generate point cloud in this case
            break;
        case 'u':
            //We simple ignore since we only generate point cloud in this case
            break;
        case 's':
            // There's a smoothing command.  We're ignoring these.
            break;
        case 'f': // We found a facet!
            //Ensure the 'f' line has at least three entries. 
            if(!IsValidFLine())
            {
                continue;
            }

            // Even though there are multiple different formats for 'f' lines. 
            //    We only need to load the vertex!	
            char vertToken[128];
            int vIdx;

            // Add this data to m_objTri
            curTri = new STriangle();
            // Read 1st
            this->GetToken( vertToken );
            sscanf( vertToken, "%d", &vIdx );
            curTri->vIdx[0] = vIdx-1;//*wavefront obj use index from 1 to n rather than starting from 0*
            // Read 2nd
            this->GetToken( vertToken );
            sscanf( vertToken, "%d", &vIdx );
            curTri->vIdx[1] = vIdx-1;
            // Read 3rd
            this->GetToken( vertToken );
            sscanf( vertToken, "%d", &vIdx );
            curTri->vIdx[2] = vIdx-1;


            m_objTri.Add( curTri );

            // Do we have more vertices in this facet?  Check to see if the 
            //    next token is non-empty (if empty the [0]'th char == 0)
            //    If we do, we'll have to triangulate the facet, so make a new tri
            this->GetToken( vertToken );
            while ( vertToken[0] ) 
            {
                //Previous triangle's two vertex plus the new vertex
                STriangle* prevTri = curTri;
                curTri = new STriangle();
                curTri->vIdx[0] = prevTri->vIdx[0];
                curTri->vIdx[1] = prevTri->vIdx[2];
                sscanf( vertToken, "%d", &vIdx );
                curTri->vIdx[2] = vIdx-1;
                m_objTri.Add( curTri );
                this->GetToken( vertToken );
            }
            break;

        default:  // We have no clue what to do with this line....
            this->WarningMessage("Found corrupt line in OBJ.  Unknown keyword '%s'", keyword);
        }
    }


    // No need to keep the file hanging around open.
    CloseFile();

}

void CPointCloudReader::AdjustCenterAndSize(IGLUArray1D<vec3>& arr)
{
    //Undefine because of FLTK's windows include
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
    float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
    float maxX = FLT_MIN, maxY = FLT_MIN, maxZ = FLT_MIN;

    // Walk through vertex array, find max & min (x, y, z)
    for (int i=0; i<arr.Size(); i++)
    {
        minX = iglu::min( minX, arr[i].X());
        maxX = iglu::max( maxX, arr[i].X());
        minY = iglu::min( minY, arr[i].Y());
        maxY = iglu::max( maxY, arr[i].Y());
        minZ = iglu::min( minZ, arr[i].Z());
        maxZ = iglu::max( maxZ, arr[i].Z());
    }

    // Find the center of the object & the width of the object
    float ctrX = (m_recenter ? 0.5*(minX+maxX) : 0.0 );
    float ctrY = (m_recenter ? 0.5*(minY+maxY) : 0.0 ); 
    float ctrZ = (m_recenter ? 0.5*(minZ+maxZ) : 0.0 );
    float deltaX = 0.5*(maxX-minX), deltaY = 0.5*(maxY-minY), deltaZ = 0.5*(maxZ-minZ);
    float delta = ( m_resize ? 
        iglu::max( deltaX, iglu::max( deltaY, deltaZ ) ) :
    1 );

    // Walk through the vertex array and update the positions
    for (int i=0; i<arr.Size(); i++)
    {
        arr[i] = vec3( (arr[i].X() - ctrX ) / delta,  (arr[i].Y() - ctrY ) / delta, (arr[i].Z() - ctrZ ) / delta);
    }
}

void CPointCloudReader::ComputeTriangleArea()
{
    //Compute each triangle's area and sum them up
    // Since it's preprocessing, I'll not optimize this code
    uint sampleNumber = m_sampleNumber;
    // Create area buffer
    float areaSum = 0;
    for (int i = 0; i < m_objTri.Size(); ++i)
    {
        //Get each triangle's 3 vertices v[3]
        vec3 v[3];
        for (int j = 0; j < 3; ++j)
        {
            v[j] = m_objVerts[m_objTri[i]->vIdx[j]];
        }

        //Compute area
        float area =  (vec3(v[0] - v[1]).Cross(vec3(v[1]-v[2]))).Length()/2.0; // ||(v0 - v1)x(v1 - v2)||/2
        assert(area >= 0);
        areaSum += area;
        m_objTri[i]->area =area;
    }
    m_totalArea = areaSum;
}

vec3 CPointCloudReader::RandomTriangle( const vec3 triangleVertex[] )
{
    // Create sample for every triangle
    // Sample on triangle
    // Get the sample point's barcentric coordinates
    float sqrt_r1 = sqrt(m_randomGenerator.fRandom()), r2 = m_randomGenerator.fRandom();
    float a = 1 - sqrt_r1;
    float b = (1- r2)*sqrt_r1;
    float c = r2*sqrt_r1;
    // Compute the points based on the barycentric coordinates
    return a*triangleVertex[0] + b*triangleVertex[1] + c*triangleVertex[2];
}

void CPointCloudReader::CreateGpuVertexArray()
{
    m_vertArrPointCloud = new IGLUVertexArray();
    m_vertArrPointCloud->SetVertexArray( m_pointList.Size()*sizeof(float), m_pointList.GetData(), IGLU_STATIC|IGLU_DRAW );
    m_vertArrPointCloud->EnableAttribute( IGLU_ATTRIB_VERTEX, 3, GL_FLOAT, 3 * sizeof(float), BUFFER_OFFSET(0) );
}

void CPointCloudReader::ClearGpuVertexArray()
{
    delete m_vertArrPointCloud;
}

void CPointCloudReader::SetSampleNumber( iglu::uint sampleNumber )
{
    m_sampleNumber = sampleNumber;
    RegeneratePointCloud();
    ClearGpuVertexArray();
    CreateGpuVertexArray();
}

float* CPointCloudReader::GetRawFloatData()
{
    return m_pointList.GetData();
}

iglu::uint CPointCloudReader::GetRawFloatSize()
{
    return m_pointList.Size();
}

iglu::uint CPointCloudReader::GetPointListSize()
{
    return m_pointList.Size()/3;
}
