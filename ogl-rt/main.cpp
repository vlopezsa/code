#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

#include <windows.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <AntTweakBar.h>

#include "camera.h"
#include "thirdparty.h"
#include "graphics.h"
#include "osutil.h"
#include "scene.h"

/* Window related variables */
char *g_strAppTitle = "OpenGL CS RayTracer";
int g_winWidth = 1024;
int g_winHeight = 768;
int g_winGlutID;

/* Scene related variables */
Scene g_Scene;

/* OpenGL related variables */
GLuint renderHandle, computeHandle;

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
    static int frame=0;

    glUseProgram(computeHandle);
    glDispatchCompute(g_winWidth / 16, g_winHeight/ 16, 1);

    glUseProgram(renderHandle);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    TwDraw();

    glutSwapBuffers();

    frame++;
}

void ChangeSize(GLsizei w, GLsizei h) {
    // Prevent division by 0
    if (h == 0) h = 1;

    // Store window's width and height.
    g_winWidth = w;
    g_winHeight = h;
    // Set the viewport.
    glViewport(0, 0, w, h);

    TwWindowSize(w, h);
}

bool g_lightMap = true;

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
                std::cout << "[INFO] Model loaded" << std::endl;
                osDisplaySceneInfo(&g_Scene);
                g_Scene.buildBVH();
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

void toolBoxSetup()
{
    TwBar *bar = TwNewBar("Info & Options");
    TwDefine(" GLOBAL help='OGL CS RayTracer' ");
    TwDefine(" 'Info & Options' size='200 250' color='96 216 224' ");

    TwAddSeparator(bar, "cam separator", "group='Camera'");

    TwAddVarRW(bar, "Speed", TW_TYPE_FLOAT, &camSpeed, "group='Camera'");
    TwAddVarRW(bar, "PosX", TW_TYPE_FLOAT, &g_Camera->m_Position.x, "group='Camera'");
    TwAddVarRW(bar, "PosY", TW_TYPE_FLOAT, &g_Camera->m_Position.y, "group='Camera'");
    TwAddVarRW(bar, "PosZ", TW_TYPE_FLOAT, &g_Camera->m_Position.z, "group='Camera'");
    TwAddVarRW(bar, "Pitch", TW_TYPE_FLOAT, &g_Camera->m_PitchDegrees, "group='Camera'");
    TwAddVarRW(bar, "Heading", TW_TYPE_FLOAT, &g_Camera->m_HeadingDegrees, "group='Camera'");
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

void CleanUp()
{
    endThirdParty();
}

GLuint genRenderProg(GLuint texHandle) {
    GLuint progHandle = glCreateProgram();
    GLuint vp = glCreateShader(GL_VERTEX_SHADER);
    GLuint fp = glCreateShader(GL_FRAGMENT_SHADER);

    const char *vpSrc[] = {
        "#version 430\n",
        "in vec2 pos;\
		 out vec2 texCoord;\
		 void main() {\
			 texCoord = pos*0.5f + 0.5f;\
			 gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);\
		 }"
    };

    const char *fpSrc[] = {
        "#version 430\n",
        "uniform sampler2D srcTex;\
		 in vec2 texCoord;\
		 out vec4 color;\
		 void main() {\
			 color = vec4(texture(srcTex, texCoord).xyz, 1.0);\
		 }"
    };

    glShaderSource(vp, 2, vpSrc, NULL);
    glShaderSource(fp, 2, fpSrc, NULL);

    glCompileShader(vp);
    int rvalue;
    glGetShaderiv(vp, GL_COMPILE_STATUS, &rvalue);
    if (!rvalue) {
        fprintf(stderr, "Error in compiling vp\n");
        exit(30);
    }
    glAttachShader(progHandle, vp);

    glCompileShader(fp);
    glGetShaderiv(fp, GL_COMPILE_STATUS, &rvalue);
    if (!rvalue) {
        fprintf(stderr, "Error in compiling fp\n");
        exit(31);
    }
    glAttachShader(progHandle, fp);

    glBindFragDataLocation(progHandle, 0, "color");
    glLinkProgram(progHandle);

    glGetProgramiv(progHandle, GL_LINK_STATUS, &rvalue);
    if (!rvalue) {
        fprintf(stderr, "Error in linking sp\n");
        exit(32);
    }

    glUseProgram(progHandle);
    glUniform1i(glGetUniformLocation(progHandle, "srcTex"), 0);

    GLuint vertArray;
    glGenVertexArrays(1, &vertArray);
    glBindVertexArray(vertArray);

    GLuint posBuf;
    glGenBuffers(1, &posBuf);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf);
    float data[] = {
        -1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, data, GL_STREAM_DRAW);
    GLint posPtr = glGetAttribLocation(progHandle, "pos");
    glVertexAttribPointer(posPtr, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posPtr);

    return progHandle;
}

GLuint genComputeProg(GLuint texHandle) {
    char *csMain = "rt-main.cs";

    // Creating the compute shader, and the program object containing the shader
    GLuint progHandle = glCreateProgram();
    GLuint cs = glCreateShader(GL_COMPUTE_SHADER);

    // In order to write to a texture, we have to introduce it as image2D.
    // local_size_x/y/z layout variables define the work group size.
    // gl_GlobalInvocationID is a uvec3 variable giving the global ID of the thread,
    // gl_LocalInvocationID is the local index within the work group, and
    // gl_WorkGroupID is the work group's index

    char *csSrc = NULL;

    FILE *f = fopen(csMain, "r");
    if (!f)
    {
        std::cout << "Coulnd't load CS program " << std::endl;
        return 0;
    }

    size_t s;
    fseek(f, 0L, SEEK_END);
    s = ftell(f);
    fseek(f, 0L, SEEK_SET);

    csSrc = new char[s];
    memset(csSrc, 0, s);

    fread((void *)csSrc, s, 1, f);

    fclose(f);

//    printf("%s", csSrc);

    glShaderSource(cs, 1, &csSrc, NULL);
    glCompileShader(cs);
    int rvalue;
    glGetShaderiv(cs, GL_COMPILE_STATUS, &rvalue);
    if (!rvalue) {
        fprintf(stderr, "Error in compiling the compute shader\n");
        GLchar log[10240];
        GLsizei length;
        glGetShaderInfoLog(cs, 10239, &length, log);
        fprintf(stderr, "Compiler log:\n%s\n", log);

        delete[] csSrc;

        exit(40);
    }
    glAttachShader(progHandle, cs);

    glLinkProgram(progHandle);
    glGetProgramiv(progHandle, GL_LINK_STATUS, &rvalue);
    if (!rvalue) {
        fprintf(stderr, "Error in linking compute shader program\n");
        GLchar log[10240];
        GLsizei length;
        glGetProgramInfoLog(progHandle, 10239, &length, log);
        fprintf(stderr, "Linker log:\n%s\n", log);

        delete[] csSrc;

        exit(41);
    }
    glUseProgram(progHandle);

    glUniform1i(glGetUniformLocation(progHandle, "destTex"), 0);
    glUniform1f(glGetUniformLocation(progHandle, "winWidth"), (float)g_winWidth);
    glUniform1f(glGetUniformLocation(progHandle, "winHeight"), (float)g_winHeight);


    delete[] csSrc;

    return progHandle;
}

GLuint genTexture() {
    // We create a single float channel 512^2 texture
    GLuint texHandle;
    glGenTextures(1, &texHandle);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, g_winWidth, g_winHeight, 0, GL_RED, GL_FLOAT, NULL);

    // Because we're also using this tex as an image (in order to write to it),
    // we bind it to an image unit as well
    glBindImageTexture(0, texHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    return texHandle;
}

void loadShaders()
{
    GLuint texHandle = genTexture();
    renderHandle = genRenderProg(texHandle);
    computeHandle = genComputeProg(texHandle);
}

int main(int argc, char **argv)
{
    initThirdParty(&argc, &argv);

    atexit(&CleanUp);

    glutSetup();

    loadShaders();

    cameraSetup();

    toolBoxSetup();

    glutMainLoop();

    return 0;
}