#include <stdlib.h>
#include <windows.h>
#include <GL\glew.h>
#include <GL\glut.h>

#include <iostream>
#include <math.h>

#include "util.h"
#include "ImageLoader.h"
#include "ModelLoader.h"
#include "glCamera.h"
#include "LightModel.h"

//Camera
glCamera Cam;

// The eye position
float EyeX, EyeY, EyeZ;
// The position of the eye, specified in spherical coordinates.
float XZAng, YZAng, DistanceToCenter;

bool MouseDown = false;
int LastMouseX, LastMouseY, CurMouseX, CurMouseY;
int LastMouseButtonClicked;
const float MaxYZAng = 85.0f * 0.01745f;

int TANGENT_INDEX = 1;

// Window data
int WinWidth, WinHeight, MainWinId;

// Model
tModel model = { 0 };

// Light Model
CLightModel light;

// Shader Objects
GLhandleARB GlobalProgramObject, VertexShaderObject, FragmentShaderObject;

void UpdateEyePositionFromMouse()
{
    Point3D pt = { 0,0,0 };
    GLfloat DeltaMouse;
    GLfloat dx, dy;

    if (MouseDown == false)
        return;

    // Compute the new eye position
    switch (LastMouseButtonClicked)
    {
    case GLUT_LEFT_BUTTON:
        dx = (float)(CurMouseX - LastMouseX);
        dy = (float)(CurMouseY - LastMouseY);

        Cam.ChangeHeading(0.2f * dx);
        Cam.ChangePitch(0.2f * dy);
        break;

    case GLUT_RIGHT_BUTTON:
        // Use the up-down motion to zoom in or out of the object
        /*DistanceToCenter += (CurMouseY - LastMouseY) * 0.1f;
        // Don't let the distance become 0 or a negative number!
        if (DistanceToCenter < 1.0f)
        {
        DistanceToCenter = 1.0f;
        }*/

        break;
    }

    // Update the last position of the mouse
    LastMouseX = CurMouseX;
    LastMouseY = CurMouseY;

    // Compute the position of the eye, according to the new parameters.
    //ComputeEyePos(XZAng, YZAng, DistanceToCenter, &EyeX, &EyeY, &EyeZ);
}

GLboolean CheckGLError()
{
    GLenum lastError;
    lastError = glGetError();
    return GL_NO_ERROR == lastError;
}


void EndProgram() {
    ReleaseModel(&model);
    ImageLoader::Close();
    glutDestroyWindow(MainWinId);
    exit(0);
}

void SetCameraInitialPos()
{
    Cam.m_MaxForwardVelocity = 5.0f;
    Cam.m_MaxPitchRate = 5.0f;
    Cam.m_MaxHeadingRate = 5.0f;
    Cam.m_PitchDegrees = -2.600001f;
    Cam.m_HeadingDegrees = 49.199955f;

    Cam.m_Position.x = -9.831701f;
    Cam.m_Position.y = 6.303247f;
    Cam.m_Position.z = -37.817413f;
}

void Render() {
    tMesh Mesh;
    tVertex Vertex;

    glClearColor(0.3f, 0.3f, 0.7f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
    // Load identity matrix.
    glLoadIdentity();

    UpdateEyePositionFromMouse();

    Cam.SetPrespective();

    glUseProgram(GlobalProgramObject);

    glPushMatrix();
    glScalef(0.1f, 0.1f, 0.1f);
    glEnable(GL_CULL_FACE);

    for (int j = 0; j<model.nMesh; j++) {
        Mesh = model.mesh[j];

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < Mesh.nIndex; i += 3) {
            tPixel3 c[3] = { 0 };

            light.evaluatePRT(c, j, Mesh.index[i], Mesh.index[i+1], Mesh.index[i+2]);
            Vertex = Mesh.vertex[Mesh.index[i]];
            glNormal3f(Vertex.n.x, Vertex.n.y, Vertex.n.z);
            glColor3f(c[0].r, c[0].g, c[0].b);
            glVertex3f(Vertex.p.x, Vertex.p.y, Vertex.p.z);

            Vertex = Mesh.vertex[Mesh.index[i + 1]];
            glNormal3f(Vertex.n.x, Vertex.n.y, Vertex.n.z);
            glColor3f(c[1].r, c[1].g, c[1].b);
            glVertex3f(Vertex.p.x, Vertex.p.y, Vertex.p.z);

            Vertex = Mesh.vertex[Mesh.index[i + 2]];
            glNormal3f(Vertex.n.x, Vertex.n.y, Vertex.n.z);
            glColor3f(c[2].r, c[2].g, c[2].b);
            glVertex3f(Vertex.p.x, Vertex.p.y, Vertex.p.z);
        }

        glEnd();
    }

    glPopMatrix();

    glUseProgram(0);

    glutSwapBuffers();
}

void ChangeSize(GLsizei w, GLsizei h) {
    // Prevent division by 0
    if (h == 0) h = 1;

    // Store window's width and height.
    WinWidth = w; WinHeight = h;
    // Set the viewport.
    glViewport(0, 0, w, h);

    // Reset transform
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(40.0, (GLdouble)w / (GLdouble)h, 0.6, 20000.0);
    glMatrixMode(GL_MODELVIEW);
}


void KeyEvent(unsigned char key, int x, int y)
{
    switch (key)
    {
        // Pressing ESC terminates the program
    case 27: EndProgram(); break;
    case 'w': case 'W':
        Cam.ChangeVelocity(0.5f);
        break;
    case 's': case 'S':
        Cam.ChangeVelocity(-0.5f);
        break;
    case 'a': case 'A':
    {
        float Heading = (float)((Cam.m_HeadingDegrees - 90.0f) / 180.0f * PI);
        float x = sin(Heading);
        float z = cos(Heading);

        Cam.m_Position.x += x*0.5f;
        Cam.m_Position.z += z*0.5f;
    }
    break;

    case 'd': case 'D':
    {
        float Heading = (float)((Cam.m_HeadingDegrees + 90.0f) / 180.0f * PI);
        float x = sin(Heading);
        float z = cos(Heading);

        Cam.m_Position.x += x*0.5f;
        Cam.m_Position.z += z*0.5f;
    }
    break;

    case ' ':
        SetCameraInitialPos();
        break;
    }
}

void MouseFunc(int button, int state, int x, int y)
{
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
    CurMouseX = x;
    CurMouseY = y;
}

void IdleFunc()
{
    glutPostRedisplay();
}

void LoadShaderEngine()
{
    if (GL_TRUE != glewGetExtension("GL_ARB_fragment_shader"))
    {
        printf("GL_ARB_fragment_shader extension is not available!\n");
    }
    else
    {
        printf("Fragment shader available\n");
    }

    if (GL_TRUE != glewGetExtension("GL_ARB_vertex_shader"))
    {
        printf("GL_ARB_vertex_shader extension is not available!\n");
    }
    else
    {
        printf("Vertex shader available\n");
    }

    if (GL_TRUE != glewGetExtension("GL_ARB_shader_objects"))
    {
        printf("GL_ARB_shader_objects extension is not available!\n");
    }
    else
    {
        printf("Shader object available\n");
    }

    if (GL_TRUE != glewGetExtension("GL_EXT_framebuffer_object"))
    {
        printf("GL_EXT_framebuffer_object not available");
    }
    else
    {
        //glGenFramebuffersEXT(1, FBOs);
        CheckGLError();
    }

    VertexShaderObject = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    FragmentShaderObject = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
}

int LoadShader(char *in_FileName, GLubyte **out_ShaderSource)
{
    FILE *InputFile = fopen(in_FileName, "rb");
    int Size;
    if (NULL == InputFile)
    {
        *out_ShaderSource = NULL;
        return 0;
    }
    // Get the size of the file
    fseek(InputFile, 0, SEEK_END);
    Size = ftell(InputFile);
    printf("File %s is %d bytes long\n", in_FileName, Size);

    *out_ShaderSource = (GLubyte *)malloc(Size + 1);
    fseek(InputFile, 0, SEEK_SET);

    // Read the file
    fread(*out_ShaderSource, sizeof(char), Size, InputFile);
    // Null-terminate the string
    (*out_ShaderSource)[Size] = 0;

    // Close the file
    fclose(InputFile);
    return 1;
}

int CompilerLog(GLhandleARB ProgramObject)
{
    int blen = 0;
    int slen = 0;
    GLcharARB *compiler_log = NULL;

    if (ProgramObject == 0) return 0; // not a valid program object
    glGetObjectParameterivARB(ProgramObject, GL_OBJECT_INFO_LOG_LENGTH_ARB, &blen);

    if (blen > 1)
    {
        if ((compiler_log = (GLcharARB*)malloc(blen)) == NULL)
        {
            return -3; // out of memory!
        }

        glGetInfoLogARB(ProgramObject, blen, &slen, compiler_log);
        if (compiler_log != 0)
        {
            printf("Compiler log: \n%s\n", compiler_log);
        }
    }
    if (compiler_log) free(compiler_log);
    return 0;
}

int CompileShader(GLhandleARB ProgramObject, GLubyte *ShaderSource)
{
    GLint length = (GLint)strlen((const char*)ShaderSource);
    int compiled = 0;

    glShaderSourceARB(ProgramObject, 1, (const GLcharARB **)&ShaderSource, &length);
    glCompileShaderARB(ProgramObject);
    glGetObjectParameterivARB(ProgramObject, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
    if (compiled)
    { // compilation successful!
        printf("Shader compiled OK\n");
        return 1;
    }
    else
    { // compilation error! Check compiler log! 
        CompilerLog(ProgramObject);
        return 0;
    }
}

int LinkShader(GLhandleARB GlobalProgramObject, GLhandleARB ProgramObject)
{
    glAttachObjectARB(GlobalProgramObject, ProgramObject);

    int linked;
    glLinkProgramARB(GlobalProgramObject);
    glGetObjectParameterivARB(GlobalProgramObject, GL_OBJECT_LINK_STATUS_ARB, &linked);

    if (linked)
    { // congratulations! the program is linked!
        printf("Shader linked OK\n");
        return 1;
    }
    else
    { // Linker error
        printf("Error linking shader\n");
        CompilerLog(ProgramObject);
        return 0;
    }
}

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);

    MainWinId = glutCreateWindow("PRT Lighting");

    ImageLoader::Init();
    glewInit();
    SetCameraInitialPos();

/*    if (LoadModel("spider.irrmesh", &model))
        if (LoadModel("models/spider.irrmesh", &model))
            if (LoadModel("../models/spider.irrmesh", &model))*/

    if (LoadModel("ncasti.txt", &model))
        if (LoadModel("models/castillo/ncasti.txt", &model))
            if (LoadModel("../models/castillo/ncasti.txt", &model))
                printf("Failed to load model\n");

    /* Setting the light model */
    light.setNumberBands(2);
    light.setGeometry(&model);
    
    if (light.setLightProbeFromFile("probes/forest.png"))
        if (light.setLightProbeFromFile("../probes/forest.png"))
            printf("Couldn't load light probe file\n");

    /* Pre calculate light coefficients */
    light.computeCoefficients();

    LoadShaderEngine();

    GLubyte *VertexShaderSource;
    GLubyte *FragmentShaderSource;

    LoadShader("VertexShader.vs", &VertexShaderSource);
    LoadShader("FragmentShader.fs", &FragmentShaderSource);

    if (VertexShaderSource)
    {
        CompileShader(VertexShaderObject, VertexShaderSource);
        LinkShader(GlobalProgramObject, VertexShaderObject);
        free(VertexShaderSource);
    }

    if (FragmentShaderSource)
    {
        CompileShader(FragmentShaderObject, FragmentShaderSource);
        LinkShader(GlobalProgramObject, FragmentShaderObject);
        free(FragmentShaderSource);
    }

    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(Render);
    glutKeyboardFunc(KeyEvent);
    glutMouseFunc(MouseFunc);
    glutMotionFunc(MotionFunc);
    glutIdleFunc(IdleFunc);

    glutMainLoop();
    return 0;
}