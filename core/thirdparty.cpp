#include <windows.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <FreeImage.h>
#include <AntTweakBar.h>

#include "thirdparty.h"

void FIFErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) {
    printf("\n*** ");
    if (fif != FIF_UNKNOWN) {
        printf("%s Format\n", FreeImage_GetFormatFromFIF(fif));
    }
    printf(message);
    printf(" ***\n");
}

void initThirdParty(int *argc, char ***argv)
{
    FreeImage_Initialise();
    FreeImage_SetOutputMessage(FIFErrorHandler);

    glutInit(argc, *argv);
    glewInit();

    TwInit(TW_OPENGL, NULL);
}

void endThirdParty()
{
    FreeImage_DeInitialise();
    TwTerminate();
}