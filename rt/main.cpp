#include <iostream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <ctime>

#include <GL/glut.h>
#include <AntTweakBar.h>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#include "render.h"
#include "camera.h"
#include "graphics.h"
#include "osutil.h"
#include "scene.h"
#include "thirdparty.h"
#include "environmentprobe.h"
#include "montecarlo.h"

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
bool  g_drawLightNormals = false;
bool  g_drawRT = false;
bool  g_drawLightPos = true;
float g_rayPercentage = 10.0;

#include "triangleobj.h"

typedef struct Line {
    Vector3 o;
    Vector3 e;
    Vector3 n;
    bool hit;

    Line::Line()
    {

    }

    Line::Line(const Line &line)
    {
        o = line.o;
        e = line.e;
        n = line.n;
        hit = line.hit;
    }
}Line;

std::vector<Line> lines;

static float light_position[] = { 10.0f, 10.0f, 10.0f };
static float light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };

Sampler *mcSampler = new MonteCarlo(32 * 32);

std::mutex g_rtProgMutex;

uint32_t g_cntRTProg;
uint32_t g_cntRTTotal;

static inline float clamp(float val, float min, float max)
{
    if (val < min)
        return min;
    else if (val > max)
        return max;

    return val;
}


typedef struct pointLight
{
    Vector3 pos;
    Color3 diffuse;
} pointLight;

Color3 getSkyLight(Vector4 &pos, Vector4 &n)
{
    Color3 color;

    IntersectionInfo I;

    EnvironmentMap *envMap = g_Scene.envMap;

    for (uint32_t si = 0; si < mcSampler->numSamples; si++)
    {
        float costerm = n* mcSampler->Samples[si].Cartesian;

        // only the hemisphere pointer toward the normal
        if (costerm > 0.0f)
        {
            Vector4 bPos = pos + (mcSampler->Samples[si].Cartesian*0.001);
            Ray bRay(bPos, mcSampler->Samples[si].Cartesian);

            // only asked if current direction is not occluded
            bool hitSky = g_Scene.bvh->getIntersection(bRay, &I, true);

            if (!hitSky)
            {
                color += (envMap->getSampleDir(mcSampler->Samples[si].Cartesian) * costerm);
            }
        }
    }

    float scale = (float)(4.0f * M_PI) / (float)mcSampler->numSamples;

    color *= scale;

    return color;
}

Color3 getRadiance(Ray &ray, pointLight *light, bool skyLight=true, uint32_t bounceCnt=0)
{
    Color3 color = Color3(0.0f);

    IntersectionInfo I;

    EnvironmentMap *envMap = g_Scene.envMap;

    bool hit = g_Scene.bvh->getIntersection(ray, &I, false);

    if (hit) {
        TriangleObj *tr = (TriangleObj *)I.object;
        Mesh *m = &g_Scene.mesh[tr->meshId];
        Material *mat = &g_Scene.material[m->materialIdx];
        Vector4 n = tr->getNormal(I);
        Vector2 uv = tr->getUV(I);

        // Get Light from environment
        if (skyLight)
        {
            color += getSkyLight(I.hit, n);
        }

        // Get Light Contribution
        if (light)
        {
            Vector4 lightDir = Vector4(light->pos) - I.hit;

            float distLight = length(lightDir);

            lightDir = normalize(lightDir);

            Ray rayLight(I.hit + (lightDir*0.001f), lightDir);
            IntersectionInfo ILight;

            bool hitLight = g_Scene.bvh->getIntersection(rayLight, &ILight, false);

            if (!hitLight || (hitLight && ILight.t>distLight))
            {
                float lightCont = fmax(lightDir * n, 0.0f);

                color += (light->diffuse * lightCont);
            }
        }

        // Get Indirect Light
        if (bounceCnt > 0)
        {
            Color3  idrRadiance;
            for (uint32_t si = 0; si < mcSampler->numSamples; si++)
            {
                float costerm = n * mcSampler->Samples[si].Cartesian;

                // only the hemisphere pointer toward the normal
                if (costerm > 0.0f)
                {
                    Vector4 bPos = I.hit + (mcSampler->Samples[si].Cartesian*0.001);
                    Ray bRay(bPos, mcSampler->Samples[si].Cartesian);

                    idrRadiance += getRadiance(bRay, light, skyLight, bounceCnt - 1) * costerm;
                }
            }

            float scale = (float)(4.0f * M_PI) / (float)mcSampler->numSamples;

            idrRadiance *= scale;

            color += idrRadiance;
        }

        if (mat->getNumTexDiffuse() > 0)
            color *= g_Scene.texture.sampleTextureImage(mat->texIdx.diffuse[0], uv);
        
        color *= mat->Color.diffuse;
    }
    else
    {
        Vector3 v3d;

        v3d.x = ray.d.x;
        v3d.y = ray.d.y;
        v3d.z = ray.d.z;

        color = envMap->getSampleDir(v3d);
    }

    return color;
}

inline Ray createCameraRay(
    Vector3 &Position,
    Vector3 &camera_u, Vector3 &camera_v, 
    Vector3 &camera_dir,
    uint32_t x, uint32_t y, 
    uint32_t Width, uint32_t Height, 
    float fovy, float offX, float offY)
{
    float u = ((float)x + offX) / (float)(Width - 1) - 0.5f;
    float v = ((float)(Height - 1 - y) + offY) / (float)(Height - 1) - 0.5f;
    float fov = 0.5f / tanf(fovy * 3.14159265*.5f / 180.f);

    // This is only valid for square aspect ratio images
    Vector3 rayDir = camera_u*u + camera_v*v + camera_dir*fov;
    rayDir.normalize();
    Ray ray(Position, rayDir);

    return ray;
}

void _partialRayTrace(uint8_t *outBuf,
    uint32_t startX, uint32_t endX, 
    uint32_t startY, uint32_t endY,
    uint32_t Width, uint32_t Height,
    uint32_t pitch)
{
    uint32_t localRTCnt = 0;

    Vector3 Up = g_Camera->m_Up;
    Vector3 Position = g_Camera->m_Position;
    Vector3 camera_dir = g_Camera->m_Direction;

    pointLight light;

    light.pos = Vector3(light_position[0], light_position[1], light_position[2]);
    light.diffuse = Vector3(light_diffuse[0], light_diffuse[1], light_diffuse[2]);

    Position.z *= -1.0f;
    camera_dir.z *= -1.0f;
    Up.x *= -1.0f;
    Up.y *= -1.0f;

    camera_dir.normalize();

    // Camera tangent space
    Vector3 camera_u = cross(camera_dir, Up) * -1.0f;
    camera_u.normalize();

    Vector3 camera_v = cross(camera_u, camera_dir) * -1.0f;
    camera_v.normalize();

    std::vector<Vector2> offList;

    offList.push_back(Vector2(0.5, 0.5));
    /*offList.push_back(Vector2(0.25, 0.25));
    offList.push_back(Vector2(0.75, 0.5));
    offList.push_back(Vector2(0.25, 0.75));
    offList.push_back(Vector2(0.75, 0.75));*/

    for (size_t j = startY; j<endY && j<Height; j++) {
        size_t index = j * pitch;

        for (size_t i = startX; i<endX && i<Width; i++, index += 3) {
            Color3 outColor;

            // Using 4 samples per pixel
            for (uint32_t oi = 0; oi < offList.size(); oi++)
            {
                Ray ray = createCameraRay(
                    Position, camera_u, camera_v, camera_dir,
                    i, j, Width, Height,
                    40.0f, offList[oi].x, offList[oi].y);
                outColor += getRadiance(ray, &light, false, 0);
                //outColor += getRadiance(ray, NULL, true, 0);
            }

            outColor /= (float)offList.size();

            outBuf[index + 0] = (uint8_t)clamp((outColor.b * 255.0f), 0.0f, 255.0f);
            outBuf[index + 1] = (uint8_t)clamp((outColor.g * 255.0f), 0.0f, 255.0f);
            outBuf[index + 2] = (uint8_t)clamp((outColor.r * 255.0f), 0.0f, 255.0f);

            localRTCnt++;

            if (localRTCnt % 100 == 0)
            {
                g_rtProgMutex.lock();
                g_cntRTProg += localRTCnt;
                printf("\t\tPixels Computed: %u / %u\r",
                    g_cntRTProg, g_cntRTTotal);
                g_rtProgMutex.unlock();

                localRTCnt = 0;
            }
        }
    }

    g_rtProgMutex.lock();
    g_cntRTProg += localRTCnt;
    printf("\t\tPixels Computed: %u / %u\r",
        g_cntRTProg, g_cntRTTotal);
    g_rtProgMutex.unlock();
}

void TW_CALL rayTrace(void *)
{
    if (g_Scene.bvh == NULL)
        return;

    std::time_t t;

    t = std::time(nullptr);
    std::cout << "RT Started: " << std::put_time(std::localtime(&t), "%c") << std::endl;

    Image *img = g_SceneRT.texture.getTextureImage(0U);
    uint8_t *out = img->getData();

    // Init progress info
    g_cntRTProg  = 0;
    g_cntRTTotal = img->Width * img->Height;

    uint32_t numProc = std::thread::hardware_concurrency();
    uint32_t nThread;

    if (numProc <= 0)
        nThread = 1;
    else
        nThread = numProc-1;

#ifdef _DEBUG
    nThread = 1;
#endif

    uint32_t split = (uint32_t)ceilf((float)img->Height / (float)nThread);

    if (split <= 1)
    {
        nThread = 1;
        split = img->Height;
    }

    std::cout << "RT Workers: " << nThread << std::endl;

    std::vector<std::thread> threads;

    uint32_t k = 0;
    for (uint32_t j = 0, k = 0; j < nThread; k += split, j++) {
        threads.push_back(std::thread(_partialRayTrace, out, 0, img->Width, k, k+split, img->Width, img->Height, img->Pitch));
    }

    for (uint32_t j = 0; j < nThread; j++)
        threads[j].join();

    img->SaveToFile("output-rt.png");

    t = std::time(nullptr);
    std::cout << "RT Ended: " << std::put_time(std::localtime(&t), "%c") << std::endl;
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

        if (g_drawRays)
        {
            glVertex3f(
                lines[i].o.x, lines[i].o.y, lines[i].o.z
                );
            glVertex3f(
                lines[i].e.x, lines[i].e.y, lines[i].e.z
                );
        }

        if (g_drawLightNormals)
        {
            glColor3f(0.0f, 0.0f, 1.0f);
            glVertex3f(
                lines[i].o.x, lines[i].o.y, lines[i].o.z
                );
            glVertex3f(
                lines[i].n.x, lines[i].n.y, lines[i].n.z
                );
        }

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

    if (g_drawRT)
    {
        g_Render.renderScene(&g_SceneRT);
    }
     else
    {
        g_Render.renderScene(&g_Scene);
        if (g_drawRays || g_drawLightNormals)
            drawRays();

        if (g_drawLightPos)
        {
            glDisable(GL_LIGHTING);

            glPointSize(5.0f);
            glColor3f(light_diffuse[0], light_diffuse[1], light_diffuse[2]);

            glBegin(GL_POINTS);
            glVertex3f(light_position[0], light_position[1], light_position[2]);
            glEnd();
        }
    }

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
                //rayTrace();
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

    /*
    //theCabing
    g_Camera->m_PitchDegrees = 3.00001;
    g_Camera->m_HeadingDegrees = -12.8;

    g_Camera->m_Position.x = 5.70619f;
    g_Camera->m_Position.y = 1.33641f;
    g_Camera->m_Position.z = -14.8241f;
    */
    /*
    //test Scene01
    g_Camera->m_PitchDegrees = 23.2;
    g_Camera->m_HeadingDegrees = -37.4;

    g_Camera->m_Position.x = 9.52788f;
    g_Camera->m_Position.y = 9.29302f;
    g_Camera->m_Position.z = -13.0324f;
    */
    /*
    //classroom
    g_Camera->m_PitchDegrees = 8.6;
    g_Camera->m_HeadingDegrees = 30.2;

    g_Camera->m_Position.x = -1.00906;
    g_Camera->m_Position.y = 1.81881;
    g_Camera->m_Position.z = -3.5933;
    */

    //BlackSmith
    g_Camera->m_PitchDegrees = -1.2;
    g_Camera->m_HeadingDegrees = 160.2;

    g_Camera->m_Position.x = -1.05913;
    g_Camera->m_Position.y = 1.58041;
    g_Camera->m_Position.z = 4.78437;
}

void toolBoxSetup()
{
    TwBar *bar = TwNewBar("myBar");
    TwDefine(" GLOBAL help='PRT Simple Example' ");
    TwDefine(" myBar size='200 200' color='96 216 224' ");
    TwDefine(" myBar label='Info & Options'");

    TwAddSeparator(bar, "ray separator", "group='Rays'");
    //TwAddVarRW(bar, "Render", TW_TYPE_BOOL8, &g_drawRT, "group='Rays' true='Ray Tracing' false='OpenGL'");
    TwAddVarRW(bar, "Light Rays", TW_TYPE_BOOL8, &g_drawRays, "group='Rays'");
    TwAddVarRW(bar, "Light Normals", TW_TYPE_BOOL8, &g_drawLightNormals, "group='Rays'");
    TwAddVarRW(bar, "Rays Percentage", TW_TYPE_FLOAT, &g_rayPercentage, "group='Rays' min=0.0 max=100.0");
    TwAddButton(bar, "RayTrace", rayTrace, NULL, "group='Rays'");

    TwAddSeparator(bar, "light separator", "group='Light'");
    TwAddVarRW(bar, "Draw Position", TW_TYPE_BOOL8, &g_drawLightPos, "group='Light'");
    TwAddVarRW(bar, "Light.X", TW_TYPE_FLOAT, &light_position[0], "group='Light'");
    TwAddVarRW(bar, "Light.Y", TW_TYPE_FLOAT, &light_position[1], "group='Light'");
    TwAddVarRW(bar, "Light.Z", TW_TYPE_FLOAT, &light_position[2], "group='Light'");

    TwAddVarRW(bar, "Light.Red", TW_TYPE_FLOAT, &light_diffuse[0], "group='Light'");
    TwAddVarRW(bar, "Light.Green", TW_TYPE_FLOAT, &light_diffuse[1], "group='Light'");
    TwAddVarRW(bar, "Light.Blue", TW_TYPE_FLOAT, &light_diffuse[2], "group='Light'");

    TwAddSeparator(bar, "cam separator", "group='Camera'");
    TwAddVarRW(bar, "Speed", TW_TYPE_FLOAT, &camSpeed, "group='Camera'");
    TwAddVarRW(bar, "PosX", TW_TYPE_FLOAT, &g_Camera->m_Position.x, "group='Camera'");
    TwAddVarRW(bar, "PosY", TW_TYPE_FLOAT, &g_Camera->m_Position.y, "group='Camera'");
    TwAddVarRW(bar, "PosZ", TW_TYPE_FLOAT, &g_Camera->m_Position.z, "group='Camera'");
    TwAddVarRW(bar, "Heading", TW_TYPE_FLOAT, &g_Camera->m_HeadingDegrees, "group='Camera'");
    TwAddVarRW(bar, "Pitch", TW_TYPE_FLOAT, &g_Camera->m_PitchDegrees, "group='Camera'");

    TwDefine("myBar/Camera opened=false");
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

void sceneSetup()
{
    EnvironmentProbe *ep = new EnvironmentProbe();

    ep->setLightProbe(&g_Scene.texture, "D:\\Serious\\Doctorado\\code\\probes\\forest.png");

    g_Scene.envMap = ep;
}

int main(int argc, char **argv)
{
    initThirdParty(&argc, &argv);
    atexit(&CleanUp);

    glutSetup();

    cameraSetup();

    toolBoxSetup();

    sceneSetup();

    initRTScene();

//    rayTrace();

    // Run GLUT loop and hence the application
    glutMainLoop();

    return 0;
}