#include <windows.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <FreeImage.h>

#include "thirdparty.h"

void initThirdParty(int *argc, char ***argv)
{
    FreeImage_Initialise();
    glutInit(argc, *argv);
    glewInit();
}

void endThirdParty()
{
    FreeImage_DeInitialise();
}