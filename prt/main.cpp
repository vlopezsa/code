#include <iostream>

#include <GL/glut.h>
#include <AntTweakBar.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "camera.h"
#include "graphics.h"
#include "osutil.h"
#include "scene.h"
#include "thirdparty.h"

/* Window related variables */
char *g_strAppTitle = "Pre-Computed Radiance Transfer";
int g_winWidth  = 1024;
int g_winHeight = 768;
int g_winGlutID;

/* Scene related variables */
Scene g_Scene;

/* Camera related variables */
Camera g_Camera;
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

        g_Camera.ChangeHeading(0.2f * dx);
        g_Camera.ChangePitch(0.2f * dy);
        break;

    case GLUT_RIGHT_BUTTON:
        break;
    }

    // Update the last position of the mouse
    LastMouseX = CurMouseX;
    LastMouseY = CurMouseY;
}

/* GLUT callback functions */
void Render()
{
    glClearColor(0.1f, 0.1f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    UpdateEyePositionFromMouse();
    g_Camera.SetPrespective();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    for (unsigned int i = 0; i < g_Scene.numMeshes(); i++)
    {
        Mesh *m = &g_Scene.mesh[i];

        /* Setup Geometry */
        glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &m->vertex[0].position.d);
        glNormalPointer(GL_FLOAT, sizeof(Vertex), &m->vertex[0].normal.d);
        glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &m->vertex[0].tex);

        /* Setup Material */
        Material *mat = &g_Scene.material[m->materialIdx];
        glColor3f(mat->Color.diffuse.r, mat->Color.diffuse.g, mat->Color.diffuse.b);



        glDrawElements(GL_TRIANGLES, (GLint)m->numIndices(), GL_UNSIGNED_INT, &m->index[0]);
    }

    TwDraw();
    glutSwapBuffers();
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

    gluPerspective(40.0, (GLdouble)w / (GLdouble)h, 0.6, 20000.0);
    glMatrixMode(GL_MODELVIEW);

    TwWindowSize(w, h);
}

void KeyEvent(unsigned char key, int x, int y)
{
    if (TwEventKeyboardGLUT(key, x, y))
        return;

    switch (key)
    {
        case 'o': case 'O':
        {
            char fileName[1024] = { 0 };
            if (osOpenDlg(fileName, 1024))
            {
                try {
                    g_Scene.loadFromFile(fileName);
                }
                catch (std::exception &e)
                {
                    std::cout << e.what() << std::endl;
                }
            }
                
        }
        break;
        case 'w': case 'W':
            g_Camera.ChangeVelocity(camSpeed);
            break;
        case 's': case 'S':
            g_Camera.ChangeVelocity(camSpeed*-1.0f);
            break;
        case 'a': case 'A':
        {
            float Heading = (float)((g_Camera.m_HeadingDegrees - 90.0f) / 180.0f * M_PI);
            float x = sin(Heading);
            float z = cos(Heading);

            g_Camera.m_Position.x += x*camSpeed;
            g_Camera.m_Position.z += z*camSpeed;
        }
        break;

        case 'd': case 'D':
        {
            float Heading = (float)((g_Camera.m_HeadingDegrees + 90.0f) / 180.0f * M_PI);
            float x = sin(Heading);
            float z = cos(Heading);

            g_Camera.m_Position.x += x*camSpeed;
            g_Camera.m_Position.z += z*camSpeed;
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
    glutDisplayFunc(Render);
    glutKeyboardFunc(KeyEvent);
    glutMouseFunc(MouseFunc);
    glutMotionFunc(MotionFunc);
    glutPassiveMotionFunc(MotionFunc);
    glutIdleFunc(IdleFunc);
}

void cameraSetup()
{
    g_Camera.m_MaxForwardVelocity = 100.0f;
    g_Camera.m_MaxPitchRate = 5.0f;
    g_Camera.m_MaxHeadingRate = 5.0f;
    g_Camera.m_PitchDegrees = -2.600001f;
    g_Camera.m_HeadingDegrees = 49.199955f;

    g_Camera.m_Position.x = 0.0f;
    g_Camera.m_Position.y = 0.0f;
    g_Camera.m_Position.z = 0.0f;
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

    // Run GLUT loop and hence the application
    glutMainLoop();   

    return 0;
}