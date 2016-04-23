// Raytracer.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include "model_obj.h"
#include "bitmap.h"
#include "BalancedTree.h"

#pragma warning(disable: 4267)

#ifndef max
#define max(a,b) (a<b?b:a)

#endif

#ifndef min
#define min(a,b) (a<b?a:b)

#endif

struct TShaderMaterial
{
	float ambient[3];
	float diffuse[3];
	float specular[3];
	float shininess;        // [0 = min shininess, 1 = max shininess]
	float alpha;            // [0 = fully transparent, 1 = fully opaque]

	int ambientMapID;
	int diffuseMapID;
	int specularMapID;
	int bumpMapID;

	int spareNotUsed;
};

GLenum ProgramPrimaryRays;
GLenum VertexShaderPrimaryRays;
GLenum FragmentShaderPrimaryRays;

GLenum PositionTexture;
GLenum DirectionTexture;
GLenum IntersectionTexture;
GLenum FactorTexture;
GLenum AccumTexture;
GLenum RandomRayTexture;
GLenum RandomSecondaryTexture;
GLenum EnvMapTexture;
GLenum ColorTextureArray;

GLenum AccumFactorLoc;
GLenum NoiseMoveLoc;
GLenum NoiseMoveColorLoc;
GLenum InitNodeLoc;

GLenum vertexTextureBuffer;
GLenum vertexTexture;
GLenum indexTextureBuffer;
GLenum indexTexture;
/*
GLenum UVTextureBuffer;
GLenum UVTexture;
*/
GLenum treeTextureBuffer;
GLenum treeTexture;
GLenum MaterialTextureBuffer;
GLenum MaterialTexture;

GLuint PrimaryRayFrameBuffer;

GLenum ProgramIntersection;
GLenum VertexShaderIntersection;
GLenum FragmentShaderIntersection;

GLenum ProgramColor;
GLenum VertexShaderColor;
GLenum FragmentShaderColor;

GLenum ProgramEnvMap;
GLenum VertexShaderEnvMap;
GLenum FragmentShaderEnvMap;


GLuint *texturePointers[]={&PositionTexture,&DirectionTexture,&FactorTexture,&IntersectionTexture,&AccumTexture,&RandomRayTexture,&RandomSecondaryTexture};//&IntersectionTexture2};
char *names[]={"Position","Direction","Factor","Intersection","Accum","RandomRay","RandomSecondary"};
char *Caption = "OpenGL deferred Raytracer"; 
int currentpointer=4;
int currentNode = 1;

GLenum ProgramTextureDisplay;
GLenum VertexShaderTextureDisplay;
GLenum FragmentShaderTextureDisplay;

static int mouse_x = 0, mouse_y = 0 ;
static int button_down = GLUT_DOWN ;
const int windowtexturesize=1024;
const int texturesize=1024;
const int randomtexturesize = 1024;
int numpasses=1;
int nimages = 0;
float speed = 1.0f;
bool showfps = false;

ModelOBJ obj;
TBalancedTree Tree;

typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALFARPROC)( int );
PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;

static void Mouse ( int button, int updown, int x, int y )
{
  button_down = updown ;

  if ( updown == GLUT_DOWN )
  {
    mouse_x = x ;
    mouse_y = y ;
  }
}

static void MouseMotion ( int x, int y )
{
  glMatrixMode ( GL_MODELVIEW ) ;
  x-=mouse_x;
  y-=mouse_y;
  float angle = sqrt(float(x*x+y*y));
  float _x=float(x)/angle;
  float _y=float(y)/angle;
  glRotatef(angle,-_y,-_x,0.0);
  glutPostRedisplay();
  mouse_x += x ;
  mouse_y += y ;
  nimages = 0;
}

int checkError(const char *functionName)
{
	GLenum error;
	int numerrors=0;
	while (( error = glGetError() ) != GL_NO_ERROR) {
		fprintf (stderr, "GL error 0x%X detected in %s:\n\t%s\n", error, functionName,gluErrorString(error));
	}
	return numerrors;
}
char *textFileRead(char *fn) 
{
        FILE *fp=NULL;
		fopen_s(&fp,fn,"rt");
        char *content = NULL;
        int count;
        if (fp != NULL) {
                fseek(fp, 0, 2);
                count=ftell(fp);
                fseek(fp,0,0);
                if (count > 0) {
                        content = (char *)malloc(sizeof(char) * (count+1));
                        count = (int)fread(content,sizeof(char),count,fp);
                        content[count] = '\0';
                }
                fclose(fp);
        }
        return content;
}            


void drawQUAD(void)
{
	float W=float(glutGet(GLUT_WINDOW_WIDTH));
	float H=float(glutGet(GLUT_WINDOW_HEIGHT));
	float minWH = W<H?W:H;
	float sx = float(W)/float(minWH);
	float sy = float(H)/float(minWH);
	glBegin(GL_QUADS);
		glVertex3f(-sx,-sy,-2.0);
		glVertex3f( sx,-sy,-2.0);
		glVertex3f( sx, sy,-2.0);
		glVertex3f(-sx, sy,-2.0);
	glEnd();
	glFlush ();
}

void primaryrays(void)
{
	//assign FBO for primary rays
	glUseProgram(ProgramPrimaryRays); 
	glUniform1f(AccumFactorLoc, float(nimages-1)/float(nimages));
	glUniform2f(NoiseMoveLoc,float(rand())/float(RAND_MAX),float(rand())/float(RAND_MAX));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,AccumTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,RandomRayTexture);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, PrimaryRayFrameBuffer);
    glPushAttrib(GL_VIEWPORT_BIT); 
	glViewport(0, 0, windowtexturesize, windowtexturesize);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT , GL_COLOR_ATTACHMENT3_EXT, GL_COLOR_ATTACHMENT4_EXT };
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, PositionTexture, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, DirectionTexture, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, FactorTexture, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_2D, IntersectionTexture, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT4_EXT, GL_TEXTURE_2D, AccumTexture, 0);
	glDrawBuffers(5, buffers);
	//assign shaders
	//draw
	if (nimages==1)
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	else
		glClear (GL_DEPTH_BUFFER_BIT);
	drawQUAD();
	//restore everything
    glPopMatrix();
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void intersection(void)
{
	//assign FBO for primary rays	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, PrimaryRayFrameBuffer);
    glPushAttrib(GL_VIEWPORT_BIT); 
	glViewport(0, 0, windowtexturesize, windowtexturesize);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT};
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, IntersectionTexture, 0);
	glDrawBuffers(1, buffers);
	glUseProgram(ProgramIntersection); 
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,PositionTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,DirectionTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,IntersectionTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture( GL_TEXTURE_BUFFER_EXT,vertexTexture);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture( GL_TEXTURE_BUFFER_EXT,indexTexture);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture( GL_TEXTURE_BUFFER_EXT,treeTexture);
	glUniform1i(InitNodeLoc, currentNode);
	//assign shaders
	//draw
	glClear (GL_DEPTH_BUFFER_BIT);
	drawQUAD();
	//restore everything
    glPopMatrix();
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void color(void)
{
	//assign FBO for primary rays
	glUseProgram(ProgramColor);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,PositionTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,DirectionTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,IntersectionTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D,FactorTexture);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D,AccumTexture);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture( GL_TEXTURE_BUFFER_EXT,vertexTexture);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture( GL_TEXTURE_BUFFER_EXT,indexTexture);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D,RandomSecondaryTexture);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D_ARRAY_EXT,ColorTextureArray);
	/*
	glActiveTexture(GL_TEXTURE9);
	glBindTexture( GL_TEXTURE_BUFFER_EXT,UVTexture);
	*/
	glActiveTexture(GL_TEXTURE9);
	glBindTexture( GL_TEXTURE_BUFFER_EXT,MaterialTexture);

	glUniform2f(NoiseMoveColorLoc,float(rand())/float(RAND_MAX),float(rand())/float(RAND_MAX));
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, PrimaryRayFrameBuffer);
    glPushAttrib(GL_VIEWPORT_BIT); 
	glViewport(0, 0, windowtexturesize, windowtexturesize);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT };
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, PositionTexture, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, DirectionTexture, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, FactorTexture, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_2D, AccumTexture, 0);
	glDrawBuffers(4, buffers);
	//assign shaders
	//draw
	glClear (GL_DEPTH_BUFFER_BIT);
	drawQUAD();
	//restore everything
    glPopMatrix();
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void envmap(void)
{
	//assign FBO for primary rays
	glUseProgram(ProgramEnvMap); 
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,AccumTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,IntersectionTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,FactorTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D,DirectionTexture);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP,EnvMapTexture);
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, PrimaryRayFrameBuffer);
    glPushAttrib(GL_VIEWPORT_BIT); 
	glViewport(0, 0, windowtexturesize, windowtexturesize);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT };
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, AccumTexture, 0);
	glDrawBuffers(1, buffers);
	//assign shaders
	//draw
	glClear (GL_DEPTH_BUFFER_BIT);
	drawQUAD();
	//restore everything
    glPopMatrix();
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void display(void)
{
	nimages++;
	primaryrays();
	for (int i=0;i<numpasses;i++)
	{
		intersection();
		color();
	}
	envmap();

	glUseProgram(ProgramTextureDisplay);
	//bind desired texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,*texturePointers[currentpointer]);
	glClear (GL_DEPTH_BUFFER_BIT);
	drawQUAD();
	glutSwapBuffers();
	checkError("display");
}

void reshape (int w, int h)
{
   glViewport (0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glViewport(0, 0, w,h);
   glOrtho(-1.0, 1.0, -1.0, 1.0, 0.0, 3.0);
   glMatrixMode(GL_MODELVIEW);
   nimages=0;
}

int lastrendertime;
int lastfpstime;
int lastmovetime;
int numframes;

void move(int time)
{	
	if (time==0)
		return;
	float x=0.0,y=0.0,z=0.0;
	if (GetAsyncKeyState('W')!=0)
	{
		z-=time;
		nimages = 0;
	}
	if (GetAsyncKeyState('S')!=0)
	{
		z+=time;
		nimages = 0;
	}
	if (GetAsyncKeyState('A')!=0)
	{
		x-=time;
		nimages = 0;
	}
	if (GetAsyncKeyState('D')!=0)
	{
		x+=time;
		nimages = 0;
	}
	if (GetAsyncKeyState('Q')!=0)
	{
		y-=time;
		nimages = 0;
	}
	if (GetAsyncKeyState('E')!=0)
	{
		y+=time;
		nimages = 0;
	}
	glTranslatef(speed*float(x)/1000.0f,speed*float(y)/1000.0f,speed*float(z)/1000.0f);
}

static void idle(void)
{
	/*
	int dif = glutGet(GLUT_ELAPSED_TIME) - lastrendertime;
	while (dif<20)
	{
		Sleep(20-dif);
		dif = glutGet(GLUT_ELAPSED_TIME) - lastrendertime;
	}
	lastrendertime+=20;
	*/
	int fpstime = glutGet(GLUT_ELAPSED_TIME)-lastfpstime;
	if (fpstime>1000)
	{
		if (showfps)
			printf_s("%0.5f FPS (%0.6f SPF). %d accum frames\n",float(1000*numframes)/float(fpstime),float(fpstime)/float(1000*numframes),nimages);
		lastfpstime+=fpstime;
		numframes=0;
		//currentpointer=1;//(currentpointer+1)%(sizeof(texturePointers)/sizeof(texturePointers[0]));
	}
	int time = glutGet(GLUT_ELAPSED_TIME)-lastmovetime;
	lastmovetime+=time;
	if (GetFocus()!=NULL)
		move(time);
    glutPostRedisplay();
	numframes++;
}

int CrearCompilarShader(GLenum &Shader,GLenum shaderType,char *filePath2)
{
	Shader = glCreateShader(shaderType);

    char filePath[1024] = {};

    sprintf(filePath, "D:\\Serious\\Doctorado\\code\\bin64\\%s", filePath2);

	char *program = textFileRead(filePath);
	if (program==NULL)
	{
		printf_s("Error: Could not load shader at %s\n",filePath);
		return -1;
	}
	printf_s("Compiling %s:\n",filePath);
	glShaderSource(Shader, 1, (const GLcharARB**)&program, NULL);
	glCompileShader(Shader);
	GLint infoLogLength,status;
	GLchar *infoLog;
	glGetShaderiv (Shader, GL_COMPILE_STATUS, &status);
	glGetShaderiv (Shader, GL_INFO_LOG_LENGTH, &infoLogLength);
	infoLog = (GLchar*) malloc (infoLogLength);
	glGetShaderInfoLog (Shader, infoLogLength, NULL, infoLog);
	fprintf (stderr, "compile log: %s\n", infoLog);
	free (infoLog);
	if (status == GL_FALSE)
	{
		return -1;
	}
	return 0;
}

bool loadshaders(void)
{
	//shader initialization
	printf_s("Loading and compiling shaders...\n");
	if (CrearCompilarShader(VertexShaderPrimaryRays,GL_VERTEX_SHADER,"Shaders\\primaryrays.vert")!=0)
	{
		return false;;
	}
	if (CrearCompilarShader(FragmentShaderPrimaryRays,GL_FRAGMENT_SHADER,"Shaders\\primaryrays.frag")!=0)
	{
		return false;;
	}
	ProgramPrimaryRays= glCreateProgram();
    glAttachShader(ProgramPrimaryRays, VertexShaderPrimaryRays);
    glAttachShader(ProgramPrimaryRays, FragmentShaderPrimaryRays);
    glLinkProgram(ProgramPrimaryRays);
	glUseProgram(ProgramPrimaryRays);
	GLint Loc = glGetUniformLocation(ProgramPrimaryRays,"Accumtexture");
	glUniform1i(Loc, 0);
	Loc = glGetUniformLocation(ProgramPrimaryRays,"Jittertexture");
	glUniform1i(Loc, 1);
	AccumFactorLoc = glGetUniformLocation(ProgramPrimaryRays,"AccumFactor");
	glUniform1f(AccumFactorLoc, 0.0f);
	NoiseMoveLoc =  glGetUniformLocation(ProgramPrimaryRays,"noisemove");
	Loc=glGetUniformLocation(ProgramPrimaryRays,"texturesize");
	glUniform1f(Loc,1.0f/float(windowtexturesize));

	if (CrearCompilarShader(VertexShaderIntersection,GL_VERTEX_SHADER,"Shaders/intersection.vert")!=0)
	{
		return false;;
	}
	if (CrearCompilarShader(FragmentShaderIntersection,GL_FRAGMENT_SHADER,"Shaders/intersection.frag")!=0)
	{
		return false;;
	}
	ProgramIntersection= glCreateProgram();
    glAttachShader(ProgramIntersection, VertexShaderIntersection);
    glAttachShader(ProgramIntersection, FragmentShaderIntersection);
    glLinkProgram(ProgramIntersection);
	glUseProgram(ProgramIntersection); 
	Loc = glGetUniformLocation(ProgramIntersection,"Positiontexture");
	glUniform1i(Loc, 0);
	Loc = glGetUniformLocation(ProgramIntersection,"Directiontexture");
	glUniform1i(Loc, 1);
	Loc = glGetUniformLocation(ProgramIntersection,"Intersectiontexture");
	glUniform1i(Loc, 2);
	Loc = glGetUniformLocation(ProgramIntersection,"vertexSampler");
	glUniform1i(Loc, 3);
	Loc = glGetUniformLocation(ProgramIntersection,"indexSampler");
	glUniform1i(Loc, 4);
	Loc = glGetUniformLocation(ProgramIntersection,"treeSampler");
	glUniform1i(Loc, 5);
	InitNodeLoc = glGetUniformLocation(ProgramIntersection,"initNode");
	glUniform1i(InitNodeLoc, currentNode);

	Loc = glGetUniformLocation(ProgramIntersection,"numtriangles");
	glUniform1i(Loc,obj.getNumberOfTriangles() );

	if (CrearCompilarShader(VertexShaderColor,GL_VERTEX_SHADER,"Shaders/color.vert")!=0)
	{
		return false;;
	}
	if (CrearCompilarShader(FragmentShaderColor,GL_FRAGMENT_SHADER,"Shaders/color.frag")!=0)
	{
		return false;;
	}
	ProgramColor= glCreateProgram();
    glAttachShader(ProgramColor, VertexShaderColor);
    glAttachShader(ProgramColor, FragmentShaderColor);
    glLinkProgram(ProgramColor);
	glUseProgram(ProgramColor);
	Loc = glGetUniformLocation(ProgramColor,"Positiontexture");
	glUniform1i(Loc, 0);
	Loc = glGetUniformLocation(ProgramColor,"Directiontexture");
	glUniform1i(Loc, 1);
	Loc = glGetUniformLocation(ProgramColor,"Intersectiontexture");
	glUniform1i(Loc, 2);
	Loc = glGetUniformLocation(ProgramColor,"Factortexture");
	glUniform1i(Loc, 3);
	Loc = glGetUniformLocation(ProgramColor,"Accumtexture");
	glUniform1i(Loc, 4);
	Loc = glGetUniformLocation(ProgramColor,"vertexSampler");
	glUniform1i(Loc, 5);
	Loc = glGetUniformLocation(ProgramColor,"indexSampler");
	glUniform1i(Loc, 6);
	Loc = glGetUniformLocation(ProgramColor,"Randomtexture");
	glUniform1i(Loc, 7);
	Loc = glGetUniformLocation(ProgramColor,"texturearray");
	glUniform1i(Loc, 8);
	/*
	Loc = glGetUniformLocation(ProgramColor,"UVSampler");
	glUniform1i(Loc, 9);
	*/
	Loc = glGetUniformLocation(ProgramColor,"MaterialSampler");
	glUniform1i(Loc, 9);
	Loc = glGetUniformLocation(ProgramColor,"numtriangles");
	glUniform1i(Loc, obj.getNumberOfTriangles());
	NoiseMoveColorLoc =  glGetUniformLocation(ProgramColor,"noisemove");

	if (CrearCompilarShader(VertexShaderEnvMap,GL_VERTEX_SHADER,"Shaders/envmap.vert")!=0)
	{
		return false;;
	}
	if (CrearCompilarShader(FragmentShaderEnvMap,GL_FRAGMENT_SHADER,"Shaders/envmap.frag")!=0)
	{
		return false;;
	}
	ProgramEnvMap= glCreateProgram();
    glAttachShader(ProgramEnvMap, VertexShaderEnvMap);
    glAttachShader(ProgramEnvMap, FragmentShaderEnvMap);
    glLinkProgram(ProgramEnvMap);
	glUseProgram(ProgramEnvMap);
	Loc = glGetUniformLocation(ProgramEnvMap,"Accumexture");
	glUniform1i(Loc, 0);
	Loc = glGetUniformLocation(ProgramEnvMap,"Intersectiontexture");
	glUniform1i(Loc, 1);
	Loc = glGetUniformLocation(ProgramEnvMap,"Factortexture");
	glUniform1i(Loc, 2);
	Loc = glGetUniformLocation(ProgramEnvMap,"Directiontexture");
	glUniform1i(Loc, 3);
	Loc = glGetUniformLocation(ProgramEnvMap,"EnvMaptexture");
	glUniform1i(Loc, 4);

	if (CrearCompilarShader(VertexShaderTextureDisplay,GL_VERTEX_SHADER,"Shaders/TextureDisplay.vert")!=0)
	{
		return false;;
	}
	if (CrearCompilarShader(FragmentShaderTextureDisplay,GL_FRAGMENT_SHADER,"Shaders/TextureDisplay.frag")!=0)
	{
		return false;;
	}
	ProgramTextureDisplay= glCreateProgram();
    glAttachShader(ProgramTextureDisplay, VertexShaderTextureDisplay);
    glAttachShader(ProgramTextureDisplay, FragmentShaderTextureDisplay);
    glLinkProgram(ProgramTextureDisplay);
    glUseProgram(ProgramTextureDisplay);    
	GLint displayLoc = glGetUniformLocation(ProgramTextureDisplay,"display_texture");
	glUniform1i(displayLoc, 0);
	return true;
}

void Keyboard(unsigned char key, int x, int y)
{
	if ((key=='p')||(key=='P'))
	{
		currentpointer=(currentpointer+1)%(sizeof(texturePointers)/sizeof(texturePointers[0]));
		glutSetWindowTitle(names[currentpointer]);
	}
	else if ((key=='o')||(key=='O'))
	{
		int numtextures=sizeof(texturePointers)/sizeof(texturePointers[0]);
		currentpointer=(currentpointer+numtextures-1)%numtextures;
		glutSetWindowTitle(names[currentpointer]);
	}
	else if (key=='+')
	{
		numpasses++;
		printf("Changed number of passes to %d\n",numpasses);
	}
	else if (key=='-')
	{
		if (numpasses>1) {
			numpasses--;
			printf("Changed number of passes to %d\n",numpasses);
		}
	}
	else if ((key=='u')||(key=='U'))
	{
		if (currentNode>1)
		{
			currentNode>>=1;
			printf("Changing to parent node (%d)\n",currentNode);
		}
	}
	else if ((key=='j')||(key=='J'))
	{
		currentNode<<=1;
		if (Tree.getnumnodes()<=currentNode)
		{
			currentNode>>=1;
		}
		else
		{
			printf("Changing to first son (%d)\n",currentNode);
		}
	}
	else if ((key=='k')||(key=='K'))
	{
		currentNode^=1;
		if ((currentNode==0)||(currentNode>=Tree.getnumnodes()))
		{
			currentNode^=1;
		}
		else
		{
			printf("Changing to brother node (%d)\n",currentNode);
		}
	}
	else if ((key=='r')||(key=='R'))
	{
		nimages=0;
	}
	else if ((key=='t')||(key=='T'))
	{
		speed+=0.1f;
	}
	else if ((key=='g')||(key=='G'))
	{
		if (speed>0.1f)
			speed-=0.1f;
	}
	else if ((key=='l')||(key=='L'))
	{
		loadshaders();
		nimages=0;
	}
	else if (key=='0')
	{
		glMatrixMode ( GL_MODELVIEW ) ;
		glLoadIdentity();
		glTranslatef(0.0f,0.0f,1.2f);
		nimages=0;
	}
	else if (key=='1')
	{
		glMatrixMode ( GL_MODELVIEW ) ;
		glLoadIdentity();
		glTranslatef(0.8f,0.24f,0.96f);
		glRotatef(35.0f,0.0f,1.0f,0.0f);
		glRotatef(-13.0f,1.0f,0.0f,0.0f);
		nimages=0;
	}
	else if (key=='2')
	{
		glMatrixMode ( GL_MODELVIEW ) ;
		glLoadIdentity();
		glTranslatef(0.3f,0.25f,0.0f);
		glRotatef(90.0f,0.0f,1.0f,0.0f);
		glRotatef(-23.0f,1.0f,0.0f,0.0f);
		nimages=0;
	}
	else if (key=='3')
	{
		glMatrixMode ( GL_MODELVIEW ) ;
		glLoadIdentity();
		glTranslatef(-0.12f,-0.13f,0.0f);
		glRotatef(90.0f,0.0f,1.0f,0.0f);
		//glRotatef(-23.0f,1.0f,0.0f,0.0f);
		nimages=0;
	}
	else if ((key=='f')||(key=='F'))
	{
		showfps =!showfps;
	}
	else
	{
	}
	glutPostRedisplay();
}

#include "osutil.h"

int main(int argc, char* argv[])
{
	printf("%s\n",Caption);

	//read model obj
	printf("Reading model...\n");

    char fileName[1024] = { 0 };
    osOpenDlg(fileName, 1024);

    //printf("%s\n", fileName);

	if (!obj.import(fileName,false))
    //if (!obj.import("D:\\Serious\\Doctorado\\code\\bin64\\Models\\ring.obj", false))
	{
		printf_s("Error reading obj: %s\n", strerror(errno));
		exit(0);
	}
	else
	{
		printf_s("Model succesfully loaded, it contains %d vertices and %d triangles and %d materials\n",obj.getNumberOfVertices(),obj.getNumberOfTriangles(),obj.getNumberOfMaterials());
	}

	//glut initialization and setup
	glutInit(&argc, argv);
	glutInitWindowSize (max(512,windowtexturesize), max(512,windowtexturesize)); 
	glutInitWindowPosition (0, 0);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE );
	glutCreateWindow (Caption);
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	glutMouseFunc(Mouse);
	glutMotionFunc(MouseMotion);
	glutKeyboardFunc(Keyboard);
///	glutSetOption ( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION ) ;
	glutPushWindow();

    printf_s ("Vendor: %s\n", glGetString (GL_VENDOR));
    printf_s ("Renderer: %s\n", glGetString (GL_RENDERER));
    printf_s ("Version: %s\n", glGetString (GL_VERSION));
    printf_s ("GLSL: %s\n", glGetString (GL_SHADING_LANGUAGE_VERSION));

	//glew specific code
	printf_s("Initializing GLEW: ");
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		printf_s("Error: %s\n",glewGetErrorString(err));
		exit(0);
	}
	printf_s("Using GLEW %s\n",glewGetString(GLEW_VERSION));
	if (!glewIsSupported("GL_ARB_vertex_program"))
	{
		printf_s("Error: There is no support for vertex programs\n");
		exit(0);
	}
	if (!glewIsSupported("GL_ARB_fragment_program"))
	{
		printf_s("Error: There is no support for fragment programs\n");
		exit(0);		
	}
    GLint maxbuffers;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxbuffers);
    printf_s("Max color attachments: %d\n",maxbuffers);

    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxbuffers);
    printf_s("Max draw buffers: %d\n",maxbuffers);

    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS_EXT, &maxbuffers);
    printf_s("Max texture array layers: %d\n",maxbuffers);

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxbuffers);
    printf_s("Max texture image units: %d\n",maxbuffers);

    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxbuffers);
    printf_s("Max combined texture units: %d\n",maxbuffers);

	glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE_ARB, &maxbuffers);
    printf_s("Max texture buffer size: %d\n",maxbuffers);

	float objlength= obj.getLength();
	//obj.normalize(objlength,true);
	obj.normalize();
	//copy geometric info
	float *vertices=new float[8*obj.getNumberOfVertices()];
	//float *UV=new float[2*obj.getNumberOfVertices()];
	int numvertices = obj.getNumberOfVertices();
	int numtriangles = obj.getNumberOfTriangles();	
	int *indices=new int[4*numtriangles];
	int *aux=(int*)obj.getIndexBuffer();
	int meshes = obj.getNumberOfMeshes();
	for (int i=obj.getNumberOfVertices()-1;i>=0;i--)
	{
		ModelOBJ::Vertex v = obj.getVertex(i);
		vertices[i*4+0]=v.position[0];
		vertices[i*4+1]=v.position[1];
		vertices[i*4+2]=v.position[2];
		vertices[i*4+3]=1.0f;
		/*
		UV[i*2+0]=v.texCoord[0];
		UV[i*2+1]=v.texCoord[1];
		*/
	}

	//create the tree
	printf("Building tree...");
	int timestart = glutGet(GLUT_ELAPSED_TIME);
	int *sorted = new int[numtriangles];
	Tree.BuildTree(vertices,obj.getIndexBuffer(),obj.getNumberOfTriangles(),sorted);
	int buildtime = glutGet(GLUT_ELAPSED_TIME)-timestart;
	printf(" done in %d ms\n",buildtime);
	
	//copy materials
	std::map<std::string,int> TextureID;
	TextureID[""]=-1;
	std::vector<std::string> texturenames;
	//texturenames.push_back("");
	for (int i=0;i<obj.getNumberOfMaterials();i++)
	{
		const ModelOBJ::Material &Material = obj.getMaterial(i);
		// add textures to texture list
		if (TextureID.count(Material.ambientMapFilename)==0)//ambient
		{
			//add to the list
			TextureID[Material.ambientMapFilename]=texturenames.size();
			texturenames.push_back(Material.ambientMapFilename);
		}
		if (TextureID.count(Material.diffuseMapFilename)==0)//diffuse
		{
			//add to the list
			TextureID[Material.diffuseMapFilename]=texturenames.size();
			texturenames.push_back(Material.diffuseMapFilename);
		}
		if (TextureID.count(Material.specularMapFilename)==0)//specular
		{
			//add to the list
			TextureID[Material.specularMapFilename]=texturenames.size();
			texturenames.push_back(Material.specularMapFilename);
		}
		if (TextureID.count(Material.bumpMapFilename)==0)//bump
		{
			//add to the list
			TextureID[Material.bumpMapFilename]=texturenames.size();
			texturenames.push_back(Material.bumpMapFilename);
		}
	}

	// assign to every primitive its material id

	int current=0;	
	int *idmaterial=new int[numtriangles];
	int nummaterials=obj.getNumberOfMaterials();

	for (int m=0;m<meshes;m++)
	{
		ModelOBJ::Mesh const &Mesh = obj.getMesh(m);
		int materialid;
		for (materialid=0;materialid<nummaterials;materialid++)
		{
			if (Mesh.pMaterial==&(obj.getMaterial(materialid)))
			{
				break;
			}
		}
		if (materialid>=nummaterials)
		{
			materialid=0;
		}
		//int texturenumber = TextureID[Mesh.pMaterial->diffuseMapFilename];
		for (int i=0;i<Mesh.triangleCount;i++)
		{
			int id = Mesh.startIndex+(i*3);
			idmaterial[current]=materialid;
			current++;
		}
	}

	// and prepare material buffer to be sent to shader
	TShaderMaterial *Material = new TShaderMaterial[nummaterials>0?nummaterials:1];
	for (int i=0;i<nummaterials;i++)
	{
		const ModelOBJ::Material *objMaterial=&obj.getMaterial(i);

		Material[i].ambient[0]=objMaterial->ambient[0];
		Material[i].ambient[1]=objMaterial->ambient[1];
		Material[i].ambient[2]=objMaterial->ambient[2];

		Material[i].diffuse[0]=objMaterial->diffuse[0];
		Material[i].diffuse[1]=objMaterial->diffuse[1];
		Material[i].diffuse[2]=objMaterial->diffuse[2];

		Material[i].specular[0]=objMaterial->specular[0];
		Material[i].specular[1]=objMaterial->specular[1];
		Material[i].specular[2]=objMaterial->specular[2];

		Material[i].shininess = objMaterial->shininess;

		Material[i].alpha = objMaterial->alpha;

		Material[i].ambientMapID = TextureID[objMaterial->ambientMapFilename];
		Material[i].diffuseMapID = TextureID[objMaterial->diffuseMapFilename];
		Material[i].specularMapID = TextureID[objMaterial->specularMapFilename];
		Material[i].bumpMapID = TextureID[objMaterial->bumpMapFilename];

		Material[i].spareNotUsed = 0;
	}
	if (nummaterials==0)
	{
		Material[0].ambient[0]=0.0f;
		Material[0].ambient[1]=0.0f;
		Material[0].ambient[2]=0.0f;

		Material[0].diffuse[0]=0.7f;
		Material[0].diffuse[1]=0.7f;
		Material[0].diffuse[2]=0.7f;

		Material[0].specular[0]=0.8f;
		Material[0].specular[1]=0.8f;
		Material[0].specular[2]=0.8f;

		Material[0].shininess = 10.0f;

		Material[0].alpha = 1.0f;

		Material[0].ambientMapID = -1;
		Material[0].diffuseMapID = -1;
		Material[0].specularMapID = -1;
		Material[0].bumpMapID = -1;

		Material[0].spareNotUsed = 0;
	}

	for (int i=0;i<numtriangles;i++)
	{
		int id = sorted[i]*3;
		indices[i*4+0]=aux[id+0];
		indices[i*4+1]=aux[id+1];
		indices[i*4+2]=aux[id+2];
		indices[i*4+3]=idmaterial[sorted[i]];
	}

	delete[] idmaterial;

	//shader initialization
	if (loadshaders()==false)
	{
		exit(0);
	}

	//create textures and FBO
	//RGBA32 2D texture
	printf_s("Creating intgernal textures...");
	glGenTextures(1, &PositionTexture);
	glBindTexture(GL_TEXTURE_2D, PositionTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowtexturesize, windowtexturesize, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &DirectionTexture);
	glBindTexture(GL_TEXTURE_2D, DirectionTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowtexturesize, windowtexturesize, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &FactorTexture);
	glBindTexture(GL_TEXTURE_2D, FactorTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowtexturesize, windowtexturesize, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &AccumTexture);
	glBindTexture(GL_TEXTURE_2D, AccumTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowtexturesize, windowtexturesize, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &RandomRayTexture);
	glBindTexture(GL_TEXTURE_2D, RandomRayTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//generate random texture
	float *texturedata = new float[randomtexturesize*randomtexturesize*4];
	for (int p=(randomtexturesize*randomtexturesize)-1;p>=0;p--)
	{
		texturedata[p*4+0]=1.5f*((2.0f*float(rand())/float(RAND_MAX))-1.0f)/float(windowtexturesize);
		texturedata[p*4+1]=1.5f*((2.0f*float(rand())/float(RAND_MAX))-1.0f)/float(windowtexturesize);
		//generate a random point in the lens circle
		float norm;
		do
		{
			texturedata[p*4+2]=(2.0f*float(rand())/float(RAND_MAX))-1.0f;
			texturedata[p*4+3]=(2.0f*float(rand())/float(RAND_MAX))-1.0f;
			norm=texturedata[p*4+2]*texturedata[p*4+2]+texturedata[p*4+3]*texturedata[p*4+3];
		}
		while (norm>1.0);
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, randomtexturesize, randomtexturesize, 0, GL_RGBA, GL_FLOAT, texturedata);

	glGenTextures(1, &RandomSecondaryTexture);
	glBindTexture(GL_TEXTURE_2D, RandomSecondaryTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//generate random texture
	for (int p=(randomtexturesize*randomtexturesize)-1;p>=0;p--)
	{
		float norm;
		do 
		{
			texturedata[p*4+0]=((2.0f*float(rand())/float(RAND_MAX))-1.0f);
			texturedata[p*4+1]=((2.0f*float(rand())/float(RAND_MAX))-1.0f);
			texturedata[p*4+2]=((2.0f*float(rand())/float(RAND_MAX))-1.0f);
			texturedata[p*4+3]=((1.0f*float(rand())/float(RAND_MAX)));
			norm=texturedata[p*4+0]*texturedata[p*4+0]+
			     texturedata[p*4+1]*texturedata[p*4+1]+
			     texturedata[p*4+2]*texturedata[p*4+2];
		}while ((norm>1.0f)||(norm<0.001));
		norm=1.0f/sqrt(norm);
		texturedata[p*4+0]*=norm;
		texturedata[p*4+1]*=norm;
		texturedata[p*4+2]*=norm;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, randomtexturesize, randomtexturesize, 0, GL_RGBA, GL_FLOAT, texturedata);
	printf_s(" done\n");

	printf_s("Loading cube map...");
	glGenTextures(1, &EnvMapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, EnvMapTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	delete[] texturedata;
	texturedata = new float[texturesize*texturesize*4];
	Bitmap bmp;

	if (bmp.loadPicture(L"textures/posx.jpg"))
	{
		gluScaleImage(GL_BGRA,bmp.width,bmp.height,GL_UNSIGNED_BYTE,bmp.getPixels(),texturesize,texturesize,GL_FLOAT,texturedata);
		for (int r=0;r<texturesize;r++)
			for (int c=0;c<texturesize;c++)
			{
				//revert Rand B chanels
				float aux = texturedata[4*(texturesize*r+c)+0];
				texturedata[4*(texturesize*r+c)+0]=texturedata[4*(texturesize*r+c)+2];
				texturedata[4*(texturesize*r+c)+2]=aux;
			}
	}
	else
	{
		for (int r=0;r<texturesize;r++)
			for (int c=0;c<texturesize;c++)
			{
				texturedata[4*(texturesize*r+c)+0]=1.0f;
				texturedata[4*(texturesize*r+c)+1]=1.0f;
				texturedata[4*(texturesize*r+c)+2]=1.0f;
				texturedata[4*(texturesize*r+c)+3]=1.0f;
			}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA32F, texturesize, texturesize, 0, GL_RGBA, GL_FLOAT, texturedata);

	if (bmp.loadPicture(L"textures/negx.jpg"))
	{
		gluScaleImage(GL_BGRA,bmp.width,bmp.height,GL_UNSIGNED_BYTE,bmp.getPixels(),texturesize,texturesize,GL_FLOAT,texturedata);
		for (int r=0;r<texturesize;r++)
			for (int c=0;c<texturesize;c++)
			{
				//revert Rand B chanels
				float aux = texturedata[4*(texturesize*r+c)+0];
				texturedata[4*(texturesize*r+c)+0]=texturedata[4*(texturesize*r+c)+2];
				texturedata[4*(texturesize*r+c)+2]=aux;
			}
	}
	else
	{
		for (int r=0;r<texturesize;r++)
			for (int c=0;c<texturesize;c++)
			{
				texturedata[4*(texturesize*r+c)+0]=1.0f;
				texturedata[4*(texturesize*r+c)+1]=1.0f;
				texturedata[4*(texturesize*r+c)+2]=1.0f;
				texturedata[4*(texturesize*r+c)+3]=1.0f;
			}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA32F, texturesize, texturesize, 0, GL_RGBA, GL_FLOAT, texturedata);

	if (bmp.loadPicture(L"textures/posy.jpg"))
	{
		gluScaleImage(GL_BGRA,bmp.width,bmp.height,GL_UNSIGNED_BYTE,bmp.getPixels(),texturesize,texturesize,GL_FLOAT,texturedata);
		for (int r=0;r<texturesize;r++)
			for (int c=0;c<texturesize;c++)
			{
				//revert Rand B chanels
				float aux = texturedata[4*(texturesize*r+c)+0];
				texturedata[4*(texturesize*r+c)+0]=texturedata[4*(texturesize*r+c)+2];
				texturedata[4*(texturesize*r+c)+2]=aux;
			}
	}
	else
	{
		for (int r=0;r<texturesize;r++)
			for (int c=0;c<texturesize;c++)
			{
				texturedata[4*(texturesize*r+c)+0]=1.0f;
				texturedata[4*(texturesize*r+c)+1]=1.0f;
				texturedata[4*(texturesize*r+c)+2]=1.0f;
				texturedata[4*(texturesize*r+c)+3]=1.0f;
			}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA32F, texturesize, texturesize, 0, GL_RGBA, GL_FLOAT, texturedata);

	if (bmp.loadPicture(L"textures/negy.jpg"))
	{
		gluScaleImage(GL_BGRA,bmp.width,bmp.height,GL_UNSIGNED_BYTE,bmp.getPixels(),texturesize,texturesize,GL_FLOAT,texturedata);
		for (int r=0;r<texturesize;r++)
			for (int c=0;c<texturesize;c++)
			{
				//revert Rand B chanels
				float aux = texturedata[4*(texturesize*r+c)+0];
				texturedata[4*(texturesize*r+c)+0]=texturedata[4*(texturesize*r+c)+2];
				texturedata[4*(texturesize*r+c)+2]=aux;
			}
	}
	else
	{
		for (int r=0;r<texturesize;r++)
			for (int c=0;c<texturesize;c++)
			{
				texturedata[4*(texturesize*r+c)+0]=1.0f;
				texturedata[4*(texturesize*r+c)+1]=1.0f;
				texturedata[4*(texturesize*r+c)+2]=1.0f;
				texturedata[4*(texturesize*r+c)+3]=1.0f;
			}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA32F, texturesize, texturesize, 0, GL_RGBA, GL_FLOAT, texturedata);

	if (bmp.loadPicture(L"textures/posz.jpg"))
	{
		gluScaleImage(GL_BGRA,bmp.width,bmp.height,GL_UNSIGNED_BYTE,bmp.getPixels(),texturesize,texturesize,GL_FLOAT,texturedata);
		for (int r=0;r<texturesize;r++)
			for (int c=0;c<texturesize;c++)
			{
				//revert Rand B chanels
				float aux = texturedata[4*(texturesize*r+c)+0];
				texturedata[4*(texturesize*r+c)+0]=texturedata[4*(texturesize*r+c)+2];
				texturedata[4*(texturesize*r+c)+2]=aux;
			}
	}
	else
	{
		for (int r=0;r<texturesize;r++)
			for (int c=0;c<texturesize;c++)
			{
				texturedata[4*(texturesize*r+c)+0]=1.0f;
				texturedata[4*(texturesize*r+c)+1]=1.0f;
				texturedata[4*(texturesize*r+c)+2]=1.0f;
				texturedata[4*(texturesize*r+c)+3]=1.0f;
			}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA32F, texturesize, texturesize, 0, GL_RGBA, GL_FLOAT, texturedata);

	if (bmp.loadPicture(L"textures/negz.jpg"))
	{
		gluScaleImage(GL_BGRA,bmp.width,bmp.height,GL_UNSIGNED_BYTE,bmp.getPixels(),texturesize,texturesize,GL_FLOAT,texturedata);
		for (int r=0;r<texturesize;r++)
			for (int c=0;c<texturesize;c++)
			{
				//revert Rand B chanels
				float aux = texturedata[4*(texturesize*r+c)+0];
				texturedata[4*(texturesize*r+c)+0]=texturedata[4*(texturesize*r+c)+2];
				texturedata[4*(texturesize*r+c)+2]=aux;
			}
	}
	else
	{
		for (int r=0;r<texturesize;r++)
			for (int c=0;c<texturesize;c++)
			{
				texturedata[4*(texturesize*r+c)+0]=1.0f;
				texturedata[4*(texturesize*r+c)+1]=1.0f;
				texturedata[4*(texturesize*r+c)+2]=1.0f;
				texturedata[4*(texturesize*r+c)+3]=1.0f;
			}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA32F, texturesize, texturesize, 0, GL_RGBA, GL_FLOAT, texturedata);
	printf_s(" done\n");

	//load textures
	printf_s("Loading materials' textures total %Iu):\n",texturenames.size());
	glGenTextures(1, &ColorTextureArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, ColorTextureArray);
	glTexParameteri( GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri( GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, 0, GL_RGBA32F, texturesize, texturesize, texturenames.size()>0?texturenames.size():1, 0, GL_RGBA, GL_FLOAT, NULL);//plus 1 because of default material
	//default color texture
	/*for (int r=0;r<texturesize;r++)
		for (int c=0;c<texturesize;c++)
		{
			texturedata[4*(texturesize*r+c)+0]=1.0f;
			texturedata[4*(texturesize*r+c)+1]=1.0f;
			texturedata[4*(texturesize*r+c)+2]=1.0f;
			texturedata[4*(texturesize*r+c)+3]=1.0f;
		}
	glTexSubImage3D( GL_TEXTURE_2D_ARRAY_EXT, 0, 0, 0, 0, texturesize, texturesize, 1, GL_RGBA, GL_FLOAT, texturedata);
	*/

	for (unsigned int i=0;i<texturenames.size();i++)
	{
		std::string FileName = obj.getPath()+texturenames[i];
		std::wstring FileW;
		FileW=L"";//.resize(' ',FileName.length()+1);
		for (unsigned int j=0;j<FileName.length();j++)
		{
			FileW+=FileName[j];
		}
		//FileW[FileName.length()]='\0';
		printf_s("\t%d: %s\n",i+1,FileName.c_str());
		if (bmp.loadPicture(FileW.c_str()))
		{
			bmp.flipVertical();
			gluScaleImage(GL_BGRA,bmp.width,bmp.height,GL_UNSIGNED_BYTE,bmp.getPixels(),texturesize,texturesize,GL_FLOAT,texturedata);
			for (int r=0;r<texturesize;r++)
				for (int c=0;c<texturesize;c++)
				{
					//revert Rand B chanels
					float aux = texturedata[4*(texturesize*r+c)+0];
					texturedata[4*(texturesize*r+c)+0]=texturedata[4*(texturesize*r+c)+2];
					texturedata[4*(texturesize*r+c)+2]=aux;
				}
		}
		else
		{
			for (int r=0;r<texturesize;r++)
				for (int c=0;c<texturesize;c++)
				{
					texturedata[4*(texturesize*r+c)+0]=1.0f;
					texturedata[4*(texturesize*r+c)+1]=1.0f;
					texturedata[4*(texturesize*r+c)+2]=1.0f;
					texturedata[4*(texturesize*r+c)+3]=1.0f;
				}
		}
		glTexSubImage3D( GL_TEXTURE_2D_ARRAY_EXT, 0, 0, 0, i, texturesize, texturesize, 1, GL_RGBA, GL_FLOAT, texturedata);
	}
	delete[] texturedata;
	printf_s("\tdone\n");

	glGenTextures(1, &IntersectionTexture);
	glBindTexture(GL_TEXTURE_2D, IntersectionTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowtexturesize, windowtexturesize, 0, GL_RGBA, GL_INT, NULL);

	// Create texture buffer objects for primitive information
	for (int i=obj.getNumberOfVertices()-1;i>=0;i--)
	{
		ModelOBJ::Vertex v = obj.getVertex(i);
		vertices[i*8+0]=v.position[0];
		vertices[i*8+1]=v.position[1];
		vertices[i*8+2]=v.position[2];
		vertices[i*8+3]=v.texCoord[0];
		vertices[i*8+4]=v.normal[0];
		vertices[i*8+5]=v.normal[1];
		vertices[i*8+6]=v.normal[2];
		vertices[i*8+7]=v.texCoord[1];
	}
    glGenBuffers( 1, &vertexTextureBuffer);
	glGenTextures( 1, &vertexTexture);
	glBindBuffer( GL_TEXTURE_BUFFER_EXT, vertexTextureBuffer);
	glBufferData( GL_TEXTURE_BUFFER_EXT, sizeof(float)*8*numvertices, vertices, GL_STATIC_DRAW);
	glBindTexture( GL_TEXTURE_BUFFER_EXT, vertexTexture);
	glTexBufferEXT( GL_TEXTURE_BUFFER_EXT, GL_RGBA32F, vertexTextureBuffer);

    glGenBuffers( 1, &indexTextureBuffer);
	glGenTextures( 1, &indexTexture);
	glBindBuffer( GL_TEXTURE_BUFFER_EXT, indexTextureBuffer);
	glBufferData( GL_TEXTURE_BUFFER_EXT, sizeof(int)*numtriangles*4, indices, GL_STATIC_DRAW);
	glBindTexture( GL_TEXTURE_BUFFER_EXT, indexTexture);
	glTexBufferEXT( GL_TEXTURE_BUFFER_EXT, GL_RGBA32I, indexTextureBuffer);

	/*
    glGenBuffers( 1, &UVTextureBuffer);
	glGenTextures( 1, &UVTextureBuffer);
	glBindBuffer( GL_TEXTURE_BUFFER_EXT, UVTextureBuffer);
	glBufferData( GL_TEXTURE_BUFFER_EXT, sizeof(float)*numvertices*2, UV, GL_STATIC_DRAW);
	glBindTexture( GL_TEXTURE_BUFFER_EXT, UVTexture);
	glTexBufferEXT( GL_TEXTURE_BUFFER_EXT, GL_RG32F, UVTextureBuffer);
	*/
	
    glGenBuffers( 1, &treeTextureBuffer);
	glGenTextures( 1, &treeTexture);
	glBindBuffer( GL_TEXTURE_BUFFER_EXT, treeTextureBuffer);
	glBufferData( GL_TEXTURE_BUFFER_EXT, sizeof(TBalancedTree::TNode)*Tree.getnumnodes(), Tree.getNodes(), GL_STATIC_DRAW);
	glBindTexture( GL_TEXTURE_BUFFER_EXT, treeTexture);
	glTexBufferEXT( GL_TEXTURE_BUFFER_EXT, GL_RGBA32F, treeTextureBuffer);

	glGenBuffers( 1, &MaterialTextureBuffer);
	glGenTextures( 1, &MaterialTexture);
	glBindBuffer( GL_TEXTURE_BUFFER_EXT, MaterialTextureBuffer);
	glBufferData( GL_TEXTURE_BUFFER_EXT, sizeof(TShaderMaterial)*(nummaterials>0?nummaterials:1), Material, GL_STATIC_DRAW);
	glBindTexture( GL_TEXTURE_BUFFER_EXT, MaterialTexture);
	glTexBufferEXT( GL_TEXTURE_BUFFER_EXT, GL_RGBA32F, MaterialTextureBuffer);

	//-------------------------
	glGenFramebuffersEXT(1, &PrimaryRayFrameBuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, PrimaryRayFrameBuffer);
	//Attach 2D texture to this FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, PositionTexture, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, DirectionTexture, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, FactorTexture, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_2D, IntersectionTexture, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT4_EXT, GL_TEXTURE_2D, AccumTexture, 0);

	GLenum status;
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	bool fine=true;
	switch(status)
	{
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            printf_s("Unsupported framebuffer format\n");
            fine= false;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            printf_s("Framebuffer incomplete, missing attachment\n");
            fine= false;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            printf_s("Framebuffer incomplete, duplicate attachment\n");
            fine= false;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            printf_s("Framebuffer incomplete, attached images must have same dimensions\n");
            fine= false;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            printf_s("Framebuffer incomplete, attached images must have same format\n");
            fine= false;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            printf_s("Framebuffer incomplete, missing draw buffer\n");
            fine= false;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            printf_s("Framebuffer incomplete, missing read buffer\n");
            fine= false;
            break;
        default:
            fine= false;
    }
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
	if (!fine)
	{
		exit(0);
	}

	numframes = 0;
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress( "wglSwapIntervalEXT" );
    wglSwapIntervalEXT(0);
	lastmovetime=lastfpstime=lastrendertime = glutGet(GLUT_ELAPSED_TIME);
	delete[] indices;
	delete[] vertices;
	//delete[] UV;
	glTranslatef(0.0f,0.0f,1.2f);
	glutPopWindow();
	glutMainLoop();
	//Sleep(500);
	return 0;
}

