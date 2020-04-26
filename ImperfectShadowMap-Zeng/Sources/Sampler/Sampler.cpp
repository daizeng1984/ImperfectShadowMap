#include "Sampler.h"
using namespace iglu;

static char DEFAULT_VERTEX_SHADER[] = {
    "#version 400\n"
    "#pragma IGLU_DISABLE  DEPTH_TEST\n"
    "#pragma IGLU_DISABLE  BLENDING\n"
    "layout(location = 0) in vec3 vertex;\n"
    "layout(location = 2) in vec2 texcoord;\n"
    "out vec2 fragTexCoord;\n"
    "void main( void ) {\n"
    "    gl_Position = vec4( vertex.xyz, 1.0f );\n"
    "    fragTexCoord = texcoord.xy;\n"
    "}\n"
};

CSampler::CSampler( char* pixShaderFileNameForDownUpSampling, iglu::IGLURenderTexture::Ptr dest_tex  )
    :m_shaderProgram(0), m_frameBuffer(0)
{
    //initialize the shader
    InitializeSampler(pixShaderFileNameForDownUpSampling);

    //Create FBO w/o depth buffer
    m_frameBuffer = new IGLUFramebuffer();
    m_frameBuffer->AttachTexture(IGLU_COLOR, dest_tex); 
}

CSampler::~CSampler()
{

}

void CSampler::SampleFrom( iglu::IGLURenderTexture::Ptr src_tex, bool usingWidthHeight, const char* texVarName)
{
    //Set all the texture parameter to NEAREST
    src_tex->SetTextureParameters( IGLU_MAG_NEAREST | IGLU_MIN_NEAREST );

    //Set render target
    m_frameBuffer->Bind();
        if (usingWidthHeight)
        {
            //Input other useful information like width, height.
            m_shaderProgram["width"] = (float) src_tex->GetWidth();
            m_shaderProgram["height"] = (float) src_tex->GetHeight();
        }
        IGLUDraw::Fullscreen( m_shaderProgram, src_tex , texVarName );

    m_frameBuffer->Unbind();

    //TODO: Set back to previous state, but we assume linear here since IGLU don't have get states right now
    src_tex->SetTextureParameters( IGLU_MAG_LINEAR | IGLU_MIN_LINEAR  );

}

void CSampler::InitializeSampler( char* pixShaderFileNameForDownUpSampling )
{
    // Allow smart pointer to delete the memory
    m_shaderProgram = 0; 

    // Load pixel shader for down sampling or up sampling
    m_shaderProgram = new IGLUShaderProgram();
    std::string pixShaderFileString("");
    LoadFileAsString(pixShaderFileNameForDownUpSampling, pixShaderFileString);
    m_shaderProgram->CreateFromString( DEFAULT_VERTEX_SHADER, (char*) (pixShaderFileString.c_str()) );
    m_shaderProgram->SetProgramDisables(IGLU_GLSL_DEPTH_TEST);
    
}

void CSampler::LoadFileAsString( const char* fileName, std::string& outputString )
{
    std::string line;
    std::ifstream myFile(fileName);

    if(!myFile.good())
    {
        std::cout << "Error opening file: " << fileName;
    }

    while(!myFile.eof())
    {
        getline(myFile, line);
        outputString = outputString + line + "\n";
    }
    myFile.close();
}

void CSampler::CopyFrom( iglu::IGLURenderTexture::Ptr src_tex )
{
    //I don't know what is the best way to make it fast, right now I just
    //   try render it to dest text

    //Set all the texture parameter to NEAREST
    src_tex->SetTextureParameters( IGLU_MAG_NEAREST | IGLU_MIN_NEAREST );
    //Set render target
    m_frameBuffer->Bind();
    IGLUDraw::Fullscreen( src_tex);
    m_frameBuffer->Unbind();

    //TODO: Set back to previous state, but we assume linear here since IGLU don't have get states right now
    src_tex->SetTextureParameters( IGLU_MAG_LINEAR | IGLU_MIN_LINEAR );
}

IGLUShaderVariable & CSampler::operator[]( const char *varName )
{
    return m_shaderProgram[varName];
}

