#ifndef __SimpleLambertianLighting_h__
#define __SimpleLambertianLighting_h__

#include <GL/glew.h>
#include <stdio.h>
#include "iglu.h"

class SimpleLambertianLighting : public iglu::IGLUDisplayMode {
public:
    SimpleLambertianLighting();
    virtual ~SimpleLambertianLighting() {}

    virtual void Initialize( int trackNum );
    virtual void Display();

    virtual const char *GetWindowTitle( void )     
    { 
        return "Simple lambertian lighting."; 
    }

    virtual const char *GetHelpDescriptor( void )  
    { 
        return ""; 
    }

private:

};


#endif // __SimpleLambertianLighting_h__
