#include <GL/glew.h>
#include <stdio.h>
// All headers are automatically included from "iglu.h"
#include "iglu.h"
// Include the multiview class we create
#include "GlobalData/GlobalData.h"
#include "MultiView/SimplePointCloudViewer.h"
#include "MultiView/GridNDCDisplay.h"
#include "MultiView/ImperfectShadowMapTest.h"
#include "MultiView/ReflectiveShadowMap.h"
#include "MultiView/RSMWithVisibility.h"
#include "MultiView/SimpleLambertianLighting.h"

using namespace iglu;

SGlobalData* g_data = 0;

int main(int argc, char** argv)
{
    // Create the global data
    g_data = new SGlobalData();

    // Create our main window
    g_data->myWin = new IGLUMultiDisplayWindow( 512, 512, 
        "Imperfecct Shadow Map",600, 600 );

    g_data->myWin->SetWindowProperties( IGLU_WINDOW_NO_RESIZE |	
        IGLU_WINDOW_DOUBLE |
        IGLU_WINDOW_REDRAW_ON_IDLE |
        IGLU_WINDOW_W_FRAMERATE); 
    g_data->myWin->SetIdleCallback( IGLUWindow::NullIdle );
    g_data->myWin->SetPreprocessOnGLInit( OpenGLInitialization );
    g_data->myWin->SetActiveMotionCallback( Motion );
    g_data->myWin->SetMouseButtonCallback( Button );
    g_data->myWin->SetKeyboardCallback( Keys );

    // Create the display modes we can render with in this demo.
    g_data->myWin->AddDisplayMode( new RSMWithVisibility() );
    g_data->myWin->AddDisplayMode( new ReflectiveShadowMap() );
    g_data->myWin->AddDisplayMode( new SimpleLambertianLighting());
    //g_data->myWin->AddDisplayMode( new ISMAreaLightViewer() );
    g_data->myWin->AddDisplayMode( new SimplePointCloudViewer());
    g_data->myWin->AddDisplayMode( new GridNDCDisplay() );
   
    // Must be called after all calls to myWin->AddDisplayMode();
    g_data->myWin->CreateWindow( argc, argv );

    // Start running our IGLU OpenGL program!
    IGLUMultiDisplayWindow::Run();
    return 0;
}
