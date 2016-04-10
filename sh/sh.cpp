#include <iostream>
#include <vector>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2     1.57079632679489661923
#endif

#define S_RINGS		64
#define S_SECTORS	64

using namespace std;

vector<GLfloat> g_vVertices;
vector<GLfloat> g_vColors;
vector<GLfloat> g_vNormals;
vector<GLfloat> g_vTexcoords;
vector<GLushort> g_vIndices;

int g_iWidth = 800;
int g_iHeight = 600;

int g_iOrder  = 0;
int g_iDegree = 0;

float g_fPos[3] = { 0.0f, 0.0f, -5.0f };
float g_fRot[3] = { 0.0f, 0.0f,  0.0f };

bool g_bMouseDown = false;
int  g_iLastMouseX;
int  g_iLastMouseY;
int  g_iCurMouseX;
int  g_iCurMouseY;
int  g_iLastMouseButtonClicked;

float sqrt_2 = (float)sqrt(2);

int factorial(int x)
{
	int res=1;
	int i;

	if (x < 1)
		return 1;

	for (i = 1 ; i <= x; i++)
		res = res*i;

	return res;
}

float K(int l, int m)
{
	double temp = ((2.0*l + 1.0)*factorial(l - m)) / (4.0*M_PI*factorial(l + m));
	return (float)sqrt(temp);
}

float P(int l, int m, float x)
{
	float pmm = 1.0;

	if (m>0) {
		float somx2 = (float)sqrt((1.0 - x)*(1.0 + x));
		float fact = 1.0;
		for (int i = 1; i <= m; i++) {
			pmm *= (-fact) * somx2;
			fact += 2.0;
		}
	}
	if (l == m) return pmm;
	float pmmp1 = x * (2.0f*m + 1.0f) * pmm;
	if (l == m + 1) return pmmp1;
	float pll = 0.0;
	for (int ll = m + 2; ll <= l; ++ll) {
		pll = (float)((2.0*ll - 1.0)*x*pmmp1 - (ll + m - 1.0)*pmm) / (ll - m);
		pmm = pmmp1;
		pmmp1 = pll;
	}
	return pll;
}

float SH(int l, int m, float theta, float phi)
{
	float res=0.0f;

	if (m == 0)
		res = K(l, 0) * P(l, 0, cos(theta));
	else if (m < 0)
		res = sqrt_2*K(l, m)*sin(-m*phi)*P(l, -m, cos(theta));
	else
		res = sqrt_2*K(l, m)*cos( m*phi)*P(l,  m, cos(theta));

	return res;
}

void UpdateWindowTitle()
{
	char buf[256] = { 0 };

	sprintf(buf, "SH Basis Function | Order %d Degree %d", g_iOrder, g_iDegree);

	glutSetWindowTitle(buf);
}

void CreateSphere(unsigned int rings, unsigned int sectors)
{
	float const R = 1.0f / (float)(rings - 1);
	float const S = 1.0f / (float)(sectors - 1);
	unsigned int r, s;

	g_vVertices.clear();
	g_vNormals.clear();
	g_vTexcoords.clear();
	g_vIndices.clear();
	g_vColors.clear();

	g_vVertices.resize(rings * sectors * 3);
	g_vNormals.resize(rings * sectors * 3);
	g_vColors.resize(rings * sectors * 3);
	g_vTexcoords.resize(rings * sectors * 2);
	std::vector<GLfloat>::iterator v = g_vVertices.begin();
	std::vector<GLfloat>::iterator n = g_vNormals.begin();
	std::vector<GLfloat>::iterator t = g_vTexcoords.begin();
	std::vector<GLfloat>::iterator c = g_vColors.begin();
	for (r = 0; r < rings; r++) 
		for (s = 0; s < sectors; s++)
		{
			float phi   = (float)(2 * M_PI * s * S);
			float theta = (float)(M_PI * r * R);

			float y = (float)sin(-M_PI_2 + theta);
			float x = (float)(cos(phi) * sin(theta));
			float z = (float)(sin(phi) * sin(theta));

			float const sh = SH(g_iOrder, g_iDegree, theta, phi);

			*t++ = s*S;
			*t++ = r*R;

			x *= fabs(sh);
			y *= fabs(sh);
			z *= fabs(sh);
			
			*v++ = x;
			*v++ = y;
			*v++ = z;

			*n++ = x;
			*n++ = y;
			*n++ = z;

			*c++ = (sh<0.0f) ? 1.0f : 0.0f;
			*c++ = (sh<0.0f) ? 0.0f : 1.0f;
			*c++ = 0.0f;
		}

	g_vIndices.resize(rings * sectors * 4);
	std::vector<GLushort>::iterator i = g_vIndices.begin();
	for (r = 0; r < rings - 1; r++) for (s = 0; s < sectors - 1; s++) {
		*i++ = r * sectors + s;
		*i++ = r * sectors + (s + 1);
		*i++ = (r + 1) * sectors + (s + 1);
		*i++ = (r + 1) * sectors + s;
	}
}

void UpdateRotation()
{
	int dx, dy;

	dx = g_iLastMouseX - g_iCurMouseX;
	dy = g_iLastMouseY - g_iCurMouseY;

	g_fRot[1] += (dx / (float)g_iWidth)  * 10.0f;
	g_fRot[0] += (dy / (float)g_iHeight) * 10.0f;
}

void RenderFunc()
{
	glClearColor(0.3f, 0.3f, 0.7f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
	glEnable(GL_COLOR_MATERIAL);

	if (g_bMouseDown)
	{
		UpdateRotation();
	}

	glLoadIdentity();

	glTranslatef(g_fPos[0], g_fPos[1], g_fPos[2]);
	glRotatef(g_fRot[0], 1.0f, 0.0f, 0.0f);
	glRotatef(g_fRot[1], 0.0f, 1.0f, 0.0f);
	glRotatef(g_fRot[2], 0.0f, 0.0f, 1.0f);

	glPushMatrix();
	glEnable(GL_CULL_FACE);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, &g_vVertices[0]);
	glNormalPointer(GL_FLOAT, 0, &g_vNormals[0]);
	glColorPointer(3, GL_FLOAT, 0, &g_vColors[0]);

	glDrawElements(GL_QUADS, (GLsizei)g_vIndices.size(), GL_UNSIGNED_SHORT, &g_vIndices[0]);
	glPopMatrix();

	glutSwapBuffers();
}

void ReShapeFunc(GLsizei w, GLsizei h) {
	if (h == 0) h = 1;

	g_iWidth = w;
	g_iHeight = h;

	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(40.0, (GLdouble)w / (GLdouble)h, 0.6, 500.0);
	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, w, h);
}

void KeyFunc(uint8_t k, int x, int y)
{
	switch (k)
	{
	case 'w': case 'W':
		g_fPos[2] += 0.2f;

		if (g_fPos[2] > -0.5f)
			g_fPos[2] = -0.5f;
		break;
	case 's': case 'S':
		g_fPos[2] -= 0.2f;

		if (g_fPos[2] < -100.0f)
			g_fPos[2] = -100.0f;
		break;
	case 'd': case 'D':
		g_fPos[0] += 0.2f;

		if (g_fPos[0] > 5.0f)
			g_fPos[0] = 5.0f;
		break;
	case 'a': case 'A':
		g_fPos[0] -= 0.2f;

		if (g_fPos[0] < -5.0f)
			g_fPos[0] = -5.0f;
		break;

	case 'l': case 'L':
		g_iOrder--;
		if (g_iOrder < 0)
			g_iOrder;
		else
		{
			CreateSphere(S_RINGS, S_SECTORS);
			UpdateWindowTitle();
		}
		break;
	case 'o': case 'O':
		g_iOrder++;
		CreateSphere(S_RINGS, S_SECTORS);
		UpdateWindowTitle();
		break;
	case 'k': case 'K':
		g_iDegree--;
		if (g_iDegree < (-g_iOrder))
			g_iDegree = -g_iOrder;
		else
		{
			CreateSphere(S_RINGS, S_SECTORS);
			UpdateWindowTitle();
		}
		break;
	case 'i': case 'I':
		g_iDegree++;
		if (g_iDegree > g_iOrder)
			g_iDegree = g_iOrder;
		else
		{
			CreateSphere(S_RINGS, S_SECTORS);
			UpdateWindowTitle();
		}
		break;
	}
}

void MouseFunc(int button, int state, int x, int y)
{
	if (GLUT_DOWN == state)
	{
		g_iCurMouseX = g_iLastMouseX = x;
		g_iCurMouseY = g_iLastMouseY = y;
		g_iLastMouseButtonClicked = button;
		g_bMouseDown = true;
	}
	else
	{
		g_bMouseDown = false;
	}
}

void MotionFunc(int x, int y)
{
	g_iCurMouseX = x;
	g_iCurMouseY = y;
}

void IdleFunc()
{
	glutPostRedisplay();
}

void RenderInit()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	UpdateWindowTitle();
}

int main(int argc, char* argv[])
{
	CreateSphere(S_RINGS, S_SECTORS);

	glutInitWindowSize(g_iWidth, g_iHeight);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("SH Basis");

	RenderInit();

	glutReshapeFunc(ReShapeFunc);
	glutDisplayFunc(RenderFunc);
	glutKeyboardFunc(KeyFunc);
	glutMouseFunc(MouseFunc);
	glutMotionFunc(MotionFunc);
	glutIdleFunc(IdleFunc);

	// start the main glut loop, no code runs after this
	glutMainLoop();
}
