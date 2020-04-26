// This is the class that read obj file and convert it to point cloud. 
//    It normally read obj as IGLUObjReader and store the raw vertex. 
//    Then convert it to point cloud according to a suggested sample 
//    number you give. You can resample the point cloud with different
//    suggested sample number and save it to file(Txt?), Notice the reader
//    will ignore the possible attributes in *.obj like material objID
//    etc. [4/24/2012 Zeng]

#ifndef __PointCloudReader_h__
#define __PointCloudReader_h__


#pragma warning( disable: 4996 )

#include "iglu.h"

// In the same way as the iglu object reader
enum EModelFlag {
    DEFAULT_STATE               = 0x0000,
    OBJ_CENTER                  = 0x0001,  // Recenter model around the origin
    OBJ_UNITIZE                 = 0x0002,  // Resize model so it ranges from [-1..1]
};


class CPointCloudReader : public iglu::IGLUFileParser, public iglu::IGLUModel
{
    struct STriangle;
public:
    //Construct from file
    CPointCloudReader(const char * fileName, iglu::uint sampleNumber = 100, int flag = DEFAULT_STATE);
    virtual ~CPointCloudReader();

    //Generate Point cloud based on the sample number,
    void RegeneratePointCloud();

    //Parse file and store raw vertex and triangle information
    void ParseRawTriangle();
    
    //Save the point cloud to file
    void Save(const char * fileName);

    //Get the float list
    float* GetRawFloatData();

    //Get the size of the data
    iglu::uint GetRawFloatSize();
    iglu::uint GetPointListSize();

    // Get GPU vertex buffer of IGLU
    iglu::IGLUVertexArray::Ptr GetGpuVertexBuffer(){return m_vertArrPointCloud;}

    // Implementation of the necessary IGLUModel virtual methods
    virtual int Draw( iglu::IGLUShaderProgram::Ptr &shader );
    int Draw( iglu::IGLUShaderProgram::Ptr &shader, unsigned int instanceNumber, bool usePointCloud = true );
    virtual bool HasVertices ( void ) const			{ return true; }
    virtual bool HasTexCoords( void ) const         { return false; }
    virtual bool HasNormals  ( void ) const         { return false; }
    virtual bool HasMatlID   ( void ) const         { return false; }
    virtual bool HasObjectID ( void ) const			{ return false; } 

    // Set sample number
    void SetSampleNumber(iglu::uint sampleNumber);

    // Get sample number
    iglu::uint GetSampleNumber(){return m_sampleNumber;}

private:
    //Checks that an f line has enough vertices to be parsed as a triangle.
    //For example, it returns false if the f line has only two entries on it
    bool IsValidFLine();

    //Adjust the center and resize the model according to the flag in construction
    void AdjustCenterAndSize(iglu::IGLUArray1D<iglu::vec3>& arr);

    //Compute area for each triangle and get the total sum of the area
    void ComputeTriangleArea();

    //Sample on triangle surface
    iglu::vec3 RandomTriangle( const iglu::vec3 triangleVertex[] );

    //Create GPU buffer
    void CreateGpuVertexArray();

    //Clear GPU buffer
    void ClearGpuVertexArray();

private:
    //Indicate whether the obj model loaded need to be recenter and resize
    bool m_recenter;
    bool m_resize;

    //Sample number
    iglu::uint m_sampleNumber;

    //Triangles total area
    float m_totalArea;

    //Point list
    iglu::IGLUArray1D<float> m_pointList;

    //Similar to the IGLUObjReader, we have raw vertex buffer stored
    iglu::IGLUArray1D<iglu::vec3> m_objVerts;

    //each triangle will have 3 index to
    iglu::IGLUArray1D<STriangle*> m_objTri;

    // Our buffer(s) storing the vertex array for this geometry
    iglu::IGLUVertexArray::Ptr m_vertArrPointCloud;

    // Random generator
    iglu::IGLURandom m_randomGenerator;
};


#endif // __PointCloudReader_h__
