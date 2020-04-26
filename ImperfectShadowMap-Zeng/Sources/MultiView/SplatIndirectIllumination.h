#ifndef __SplatIndirectIllumination_h__
#define __SplatIndirectIllumination_h__



#include <GL/glew.h>
#include <stdio.h>
#include "iglu.h"

class SplatIndirectIllumination : public iglu::IGLUDisplayMode {
public:
    SplatIndirectIllumination();
    virtual ~SplatIndirectIllumination() {}

    virtual void Initialize( int trackNum );
    virtual void Display();

    virtual const char *GetWindowTitle( void )     
    { 
        return "Splat Inidrect Illumination."; 
    }

    virtual const char *GetHelpDescriptor( void )  
    { 
        return ""; 
    }

private:

};


#endif // __SplatIndirectIllumination_h__
