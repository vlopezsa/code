#include <iostream>

#include <GL/glut.h>
#include <AntTweakBar.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "render.h"
#include "camera.h"
#include "graphics.h"
#include "osutil.h"
#include "scene.h"
#include "thirdparty.h"
#include "prt.h"
#include "montecarlo.h"
#include "environmentprobe.h"

/* Window related variables */
char *g_strAppTitle = "Pre-Computed Radiance Transfer";
int g_winWidth = 1024;
int g_winHeight = 768;
int g_winGlutID;

/* Render related variables */
Render g_Render;
PRT    g_PRT;

bool g_prtEnable = false;

/* Scene related variables */
Scene g_Scene;

/* Camera related variables */
Camera *g_Camera = &g_Scene.camera;
float camSpeed = 0.5f;
bool MouseDown = false;
int LastMouseX, LastMouseY, CurMouseX, CurMouseY;
int LastMouseButtonClicked;

void UpdateEyePositionFromMouse()
{
    Vector3 pt = { 0,0,0 };
    GLfloat dx, dy;

    if (MouseDown == false)
        return;

    switch (LastMouseButtonClicked)
    {
    case GLUT_LEFT_BUTTON:
        dx = (float)(CurMouseX - LastMouseX);
        dy = (float)(CurMouseY - LastMouseY);

        g_Camera->ChangeHeading(0.2f * dx);
        g_Camera->ChangePitch(0.2f * dy);
        break;

    case GLUT_RIGHT_BUTTON:
        break;
    }

    // Update the last position of the mouse
    LastMouseX = CurMouseX;
    LastMouseY = CurMouseY;
}

/* GLUT callback functions */

void RenderF()
{
    g_Render.clear({ 0.1f, 0.1f, 0.0f });

    // glLoadIdentity();

    UpdateEyePositionFromMouse();

    g_Render.updateCamera(g_Camera);

    g_Render.usePreComputedEnvLight(g_prtEnable);

    g_Render.renderScene(&g_Scene);

    TwDraw();

    g_Render.swapBuffers();
}

void ChangeSize(GLsizei w, GLsizei h) {
    // Prevent division by 0
    if (h == 0) h = 1;

    // Store window's width and height.
    g_winWidth = w;
    g_winHeight = h;
    // Set the viewport.
    glViewport(0, 0, w, h);

    // Reset transform
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0, (GLdouble)w / (GLdouble)h, 0.1, 20000.0);
    glMatrixMode(GL_MODELVIEW);

    TwWindowSize(w, h);
}

bool g_lightMap = true;

void KeyEvent(uint8_t key, int x, int y)
{
    if (TwEventKeyboardGLUT(key, x, y))
        return;

    switch (key)
    {
    case 'p': case 'P':
        g_prtEnable = !g_prtEnable;
        break;

    case 'l': case 'L':
        g_lightMap = !g_lightMap;
        if (g_lightMap)
            g_PRT.preComputeLight(g_Scene.envMap);
        else
            g_PRT.preComputeLight();

        g_PRT.preComputeGeomCoeff(&g_Scene);

        break;

    case 'o': case 'O':
    {
        char fileName[1024] = { 0 };
        if (osOpenDlg(fileName, 1024))
        {
            try {
                g_Scene.loadFromFile(fileName);
                std::cout << "[INFO] Model loaded" << std::endl;
                osDisplaySceneInfo(&g_Scene);
                g_Scene.buildBVH();
                std::cout << "[INFO] BVH loaded" << std::endl;
                g_PRT.preComputeGeomCoeff(&g_Scene);
                g_Render.usePreComputedEnvLight(true);
                std::cout << "[INFO] Transport coefficients computed" << std::endl;
            }
            catch (std::exception &e)
            {
                std::cout << e.what() << std::endl;
            }
        }

    }
    break;
    case 'w': case 'W':
        g_Camera->ChangeVelocity(camSpeed);
        break;
    case 's': case 'S':
        g_Camera->ChangeVelocity(camSpeed*-1.0f);
        break;
    case 'a': case 'A':
    {
        float Heading = (float)((g_Camera->m_HeadingDegrees - 90.0f) / 180.0f * M_PI);
        float x = sin(Heading);
        float z = cos(Heading);

        g_Camera->m_Position.x += x*camSpeed;
        g_Camera->m_Position.z += z*camSpeed;
    }
    break;

    case 'd': case 'D':
    {
        float Heading = (float)((g_Camera->m_HeadingDegrees + 90.0f) / 180.0f * M_PI);
        float x = sin(Heading);
        float z = cos(Heading);

        g_Camera->m_Position.x += x*camSpeed;
        g_Camera->m_Position.z += z*camSpeed;
    }
    break;

    case 'r': case 'R':
    {
        g_Camera->m_Position.x += g_Camera->m_Up.x * camSpeed;
        g_Camera->m_Position.y += g_Camera->m_Up.y * camSpeed;
        g_Camera->m_Position.z += g_Camera->m_Up.z * camSpeed;
    }
    break;

    case 'f': case 'F':
    {
        g_Camera->m_Position.x -= g_Camera->m_Up.x * camSpeed;
        g_Camera->m_Position.y -= g_Camera->m_Up.y * camSpeed;
        g_Camera->m_Position.z -= g_Camera->m_Up.z * camSpeed;
    }
    break;
    }
}

void MouseFunc(int button, int state, int x, int y)
{
    if (TwEventMouseButtonGLUT(button, state, x, y))
        return;

    if (GLUT_DOWN == state)
    {
        CurMouseX = LastMouseX = x;
        CurMouseY = LastMouseY = y;
        LastMouseButtonClicked = button;
        MouseDown = true;
    }
    else
    {
        MouseDown = false;
    }
}

void MotionFunc(int x, int y)
{
    if (TwEventMouseMotionGLUT(x, y))
        return;

    CurMouseX = x;
    CurMouseY = y;
}

void IdleFunc()
{
    glutPostRedisplay();
}

/* Application function */

void glutSetup()
{
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(g_winWidth, g_winHeight);

    g_winGlutID = glutCreateWindow(g_strAppTitle);

    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderF);
    glutKeyboardFunc(KeyEvent);
    glutMouseFunc(MouseFunc);
    glutMotionFunc(MotionFunc);
    glutPassiveMotionFunc(MotionFunc);
    glutIdleFunc(IdleFunc);
}

void cameraSetup()
{
    g_Camera->m_MaxForwardVelocity = 100.0f;
    g_Camera->m_MaxPitchRate = 5.0f;
    g_Camera->m_MaxHeadingRate = 5.0f;
    g_Camera->m_PitchDegrees = 28.8;
    g_Camera->m_HeadingDegrees = -39.6;

    g_Camera->m_Position.x = 9.09463f;
    g_Camera->m_Position.y = 9.62256f;
    g_Camera->m_Position.z = -10.9857f;
}

void toolBoxSetup()
{
    TwBar *bar = TwNewBar("Info & Options");
    TwDefine(" GLOBAL help='PRT Simple Example' ");
    TwDefine(" 'Info & Options' size='200 250' color='96 216 224' ");

    TwAddSeparator(bar, "cam separator", "group='Camera'");

    TwAddVarRW(bar, "Speed", TW_TYPE_FLOAT, &camSpeed, "group='Camera'");
    TwAddVarRW(bar, "PosX", TW_TYPE_FLOAT, &g_Camera->m_Position.x, "group='Camera'");
    TwAddVarRW(bar, "PosY", TW_TYPE_FLOAT, &g_Camera->m_Position.y, "group='Camera'");
    TwAddVarRW(bar, "PosZ", TW_TYPE_FLOAT, &g_Camera->m_Position.z, "group='Camera'");
    TwAddVarRW(bar, "Pitch", TW_TYPE_FLOAT, &g_Camera->m_PitchDegrees, "group='Camera'");
    TwAddVarRW(bar, "Heading", TW_TYPE_FLOAT, &g_Camera->m_HeadingDegrees, "group='Camera'");

    TwAddSeparator(bar, "prt separator", "group='PRT'");
    TwAddVarRW(bar, "Enable", TW_TYPE_BOOL8, &g_prtEnable, "group='PRT'");
}

void envMapSetup()
{
    EnvironmentProbe *ep = new EnvironmentProbe();

    ep->setLightProbe(&g_Scene.texture, "D:\\Serious\\Doctorado\\code\\probes\\forest.png");

    g_Scene.envMap = ep;

    g_PRT.preComputeLight(ep);
}

void sceneSetup()
{
    g_PRT.setSampler(new MonteCarlo(64 * 64));
    g_PRT.preComputeLight();

    envMapSetup();
}

void CleanUp()
{
    endThirdParty();
}

int main(int argc, char **argv)
{
    initThirdParty(&argc, &argv);
    atexit(&CleanUp);

    glutSetup();

    cameraSetup();

    toolBoxSetup();

    sceneSetup();

    // Run GLUT loop and hence the application
    glutMainLoop();

    return 0;
}