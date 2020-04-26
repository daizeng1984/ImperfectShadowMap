#ifndef __Sampler_h__
#define __Sampler_h__

#include "iglu.h"
#include <string>
#include <fstream>
#include <iostream>

class CSampler
{
public:
    //Since we cannot attach the texture to FBO and detach it in IGLU, we have to create sampler for
    //    each texture.
    CSampler(char* pixShaderFileNameForDownUpSampling, iglu::IGLURenderTexture::Ptr dest_tex);
    ~CSampler();
public:
    void SampleFrom(iglu::IGLURenderTexture::Ptr src_tex, bool usingWidthHeight = true, const char* texVarName = "inputTex");
    void CopyFrom(iglu::IGLURenderTexture::Ptr src_tex);

public:
    //For easy access to uniform shader variable, just mimic IGLU
    typedef iglu::IGLUPtr<CSampler,iglu::IGLUShaderVariable&,const char *> Ptr;
    
	iglu::IGLUShaderVariable &operator[] ( const char *varName );
    
private:
    void InitializeSampler(char* pixShaderFileNameForDownUpSampling);
    void LoadFileAsString(const char* fileName, std::string& outputString);
    //void ValidateTexture(IGLURenderTexture::Ptr src_tex, IGLURenderTexture::Ptr dest_tex);
private:
    iglu::IGLUShaderProgram::Ptr  m_shaderProgram;
    iglu::IGLUFramebuffer::Ptr  m_frameBuffer;
};

#endif // __Sampler_h__
