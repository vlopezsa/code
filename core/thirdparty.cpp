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
    GLenum err;

    FreeImage_Initialise();
    FreeImage_SetOutputMessage(FIFErrorHandler);

    glutInit(argc, *argv);

    int win= glutCreateWindow("GLEW init");
    err = glewInit();

    err = glewInit();
    if (err != GLEW_OK)
    {
        printf("GLEW Error: %s\n", glewGetErrorString(err));
    }

    glutDestroyWindow(win);

    TwInit(TW_OPENGL, NULL);
}

void endThirdParty()
{
    FreeImage_DeInitialise();
    TwTerminate();
}