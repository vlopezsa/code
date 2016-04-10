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

/* Window related variables */
char *g_strAppTitle = "Ray Tracer";
int g_winWidth = 800;
int g_winHeight = 800;
int g_winGlutID;

/* Render related variables */
Render g_Render;

/* Scene related variables */
Scene g_Scene, g_SceneRT;

/* Camera related variables */
Camera *g_Camera = &g_Scene.camera;
float camSpeed = 0.5f;
bool MouseDown = false;
int LastMouseX, LastMouseY, CurMouseX, CurMouseY;
int LastMouseButtonClicked;

/* debug options */
bool  g_drawRays = false;
float g_rayPercentage = 10.0;

#include "triangleobj.h"

typedef struct {
    Vector3 o;
    Vector3 e;
    bool hit;
}Line;

std::vector<Line> lines;

void rayTrace()
{
    if (g_Scene.bvh == NULL)
        return;

    Image *img = g_SceneRT.texture.getTextureImage(0U);
    uint8_t *out = img->getData();

    glPushMatrix();
    g_Camera->SetPerspective();
    glPopMatrix();

    Vector3 lookAt(0, 0, 0);
    Vector3 Up(0, 1, 0);
    Vector3 Position(0.001, 25, 0.001);

    Vector3 camera_dir = lookAt - Position;
    camera_dir.normalize();

    // Camera tangent space
    Vector3 camera_u = cross(camera_dir, Up);
    camera_u.normalize();

    Vector3 camera_v = cross(camera_u, camera_dir);
    camera_v.normalize();

    lines.clear();
    lines.resize(img->Height * img->Width);

    uint32_t cnt = 0;

    for (size_t j = 0; j<img->Height; j++) {
        size_t index = j * img->Pitch;


        for (size_t i = 0; i<img->Width; i++, index += 3) {
            float u = ((float)i + .5f) / (float)(img->Width - 1) - .5f;
            float v = ((float)(img->Height - 1 - j) + .5f) / (float)(img->Height - 1) - .5f;
            float fov = 0.5f / tanf(40.f * 3.14159265*.5f / 180.f);

            // This is only valid for square aspect ratio images
            Vector3 rayDir = camera_u*u + camera_v*v + camera_dir*fov;
            rayDir.normalize();
            Ray ray(Position, rayDir);

            IntersectionInfo I;

            bool hit = g_Scene.bvh->getIntersection(ray, &I, false);

            if (!hit) {
                out[index + 0] = 0;
                out[index + 1] = 26;
                out[index + 2] = 26;
            }
            else {
                out[index + 0] = (uint8_t)(I.bary.z*255.0f);
                out[index + 1] = (uint8_t)(I.bary.y*255.0f);
                out[index + 2] = (uint8_t)(I.bary.x*255.0f);
            }

            lines[cnt].o = Position;
            lines[cnt].e = Position + (rayDir * 30.0f);
            lines[cnt].hit = hit;

            cnt++;
        }
    }

    img->SaveToFile("output-rt.png");
}

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

void drawRays()
{
    glDisable(GL_LIGHTING);

    float fi = 0.0f;
    float skip = (uint32_t)(float)1.0f / (g_rayPercentage / 100.0f);

    if (skip<=0.00001) skip = 1.0f;

    glBegin(GL_LINES);

    for (uint32_t i=0, fi=0.0f; i < lines.size();)
    {
        if(lines[i].hit)
            glColor3f(0.0f, 1.0f, 0.0f);
        else
            glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(
            lines[i].o.x, lines[i].o.y, lines[i].o.z
            );
        glVertex3f(
            lines[i].e.x, lines[i].e.y, lines[i].e.z
            );

        fi += skip;
        i = (uint32_t)fi;
    }

    glEnd();
}

void RenderF()
{
    g_Render.clear({ 0.1f, 0.1f, 0.0f });

    UpdateEyePositionFromMouse();

    g_Render.updateCamera(g_Camera);

    g_Render.renderScene(&g_Scene);

    if(g_drawRays)
        drawRays();

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

    gluPerspective(40.0, (GLdouble)w / (GLdouble)h, 0.6, 20000.0);
    glMatrixMode(GL_MODELVIEW);

    TwWindowSize(w, h);
}

void KeyEvent(uint8_t key, int x, int y)
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
                std::cout << "File loaded" << std::endl;
                g_Scene.buildBVH();
                std::cout << "BVH built" << std::endl;
                rayTrace();
                osDisplaySceneInfo(&g_Scene);
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
    g_Camera->m_PitchDegrees = 18.4;
    g_Camera->m_HeadingDegrees = 45.2;

    g_Camera->m_Position.x = -15.7551;
    g_Camera->m_Position.y = 6.6482;
    g_Camera->m_Position.z = -15.7551;
}

void toolBoxSetup()
{
    TwBar *bar = TwNewBar("Info & Options");
    TwDefine(" GLOBAL help='PRT Simple Example' ");
    TwDefine(" 'Info & Options' size='200 200' color='96 216 224' ");

    TwAddSeparator(bar, "cam separator", "group='Camera'");

    TwAddVarRW(bar, "CamSpeed", TW_TYPE_FLOAT, &camSpeed, "group='Camera'");
    TwAddVarRW(bar, "CamPosX", TW_TYPE_FLOAT, &g_Camera->m_Position.x, "group='Camera'");
    TwAddVarRW(bar, "CamPosY", TW_TYPE_FLOAT, &g_Camera->m_Position.y, "group='Camera'");
    TwAddVarRW(bar, "CamPosZ", TW_TYPE_FLOAT, &g_Camera->m_Position.z, "group='Camera'");

    TwAddSeparator(bar, "ray separator", "group='Rays'");
    TwAddVarRW(bar, "Draw Rays", TW_TYPE_BOOL8, &g_drawRays, "group='Rays'");
    TwAddVarRW(bar, "Rays Percentage", TW_TYPE_FLOAT, &g_rayPercentage, "group='Rays'");
}

void CleanUp()
{
    endThirdParty();
}

void fillUpBuffer(Image *img)
{
    unsigned int x, y;
    uint8_t *buf = img->getData();

    for (y = 0; y < img->Height; y++)
    {
        for (x = 0; x < img->Width; x++)
        {
            buf[0] = (uint8_t)(((float)x / (float)img->Width) * 255.0f);
            buf[1] = (uint8_t)(((float)y / (float)img->Height) * 255.0f);
            buf[2] = (uint8_t)(((float)(img->Width-x) / (float)img->Width) * 255.0f);

            buf += 3;
        }
    }
}

void initRTScene()
{
    /* Creating our frame buffer */
    Image *img = new Image(800, 800, ImagePixelFormat::Img_Pixel_RGB, 24);
    unsigned int texIdx=g_SceneRT.texture.addTextureFromImg(img, "texFrameBuffer");

    fillUpBuffer(img);

    /* Creating material out of frame buffer */
    g_SceneRT.setNumMaterials(1);
    Material *mat = &g_SceneRT.material[0];

    mat->texIdx.diffuse.push_back(texIdx);

    /* Creating quad to display our framebuffer */
    g_SceneRT.setNumMeshes(1);
    Mesh *m = &g_SceneRT.mesh[0];

    m->materialIdx = 0;

    m->setNumVertices(4);
    m->setNumTriangles(2);

    // Vertices
    m->vertex[0].position = Vector3(-1.0f, 1.0f, 0.0f);
    m->vertex[1].position = Vector3( 1.0f, 1.0f, 0.0f);
    m->vertex[2].position = Vector3( 1.0f,-1.0f, 0.0f);
    m->vertex[3].position = Vector3(-1.0f,-1.0f, 0.0f);

    m->vertex[0].tex = Vector2(0.0f, 0.0f);
    m->vertex[1].tex = Vector2(1.0f, 0.0f);
    m->vertex[2].tex = Vector2(1.0f, 1.0f);
    m->vertex[3].tex = Vector2(0.0f, 1.0f);

    m->vertex[0].diffuse = Vector3(1.0f, 1.0f, 1.0f);
    m->vertex[1].diffuse = Vector3(1.0f, 1.0f, 1.0f);
    m->vertex[2].diffuse = Vector3(1.0f, 1.0f, 1.0f);
    m->vertex[3].diffuse = Vector3(1.0f, 1.0f, 1.0f);

    // Indices
    m->index[0] = 0;
    m->index[1] = 1;
    m->index[2] = 2;
    m->index[3] = 0;
    m->index[4] = 2;
    m->index[5] = 3;
}

int main(int argc, char **argv)
{
    initThirdParty(&argc, &argv);
    atexit(&CleanUp);

    glutSetup();

    cameraSetup();

    toolBoxSetup();

    initRTScene();

    // Run GLUT loop and hence the application
    glutMainLoop();

    return 0;
}