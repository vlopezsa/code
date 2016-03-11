// glVector.h: interface for the glVector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GLVECTOR_H__F526A5CF_89B5_4F20_8F2C_517D83879D35__INCLUDED_)
#define AFX_GLVECTOR_H__F526A5CF_89B5_4F20_8F2C_517D83879D35__INCLUDED_

#include <windows.h>		// Header File For Windows
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include <gl\glut.h>		// Header File For The Glaux Library

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "util.h"
class glVector  
{
public:
    glVector();
    glVector(float a, float b, float c);
    glVector(glVector &v);
    glVector(Point3D &p);
	virtual ~glVector();

    union
    {
        float d[3];
        struct { float x, y, z; };
        struct { float r, g, b; };
    };

    void operator *=(float scalar);
    void operator /=(float scalar);
    void operator +=(float scalar);
    void operator -=(float scalar);
    void operator +=(const glVector &v);
    void operator -=(const glVector &v);

};

glVector operator *(const glVector &v,  float scalar);
glVector operator /(const glVector &v,  float scalar);
glVector operator +(const glVector &v, float scalar);
glVector operator -(const glVector &v, float scalar);
glVector operator +(const glVector &v1, const glVector &v2);
glVector operator -(const glVector &v1, const glVector &v2);

glVector cross(const glVector &v1, const glVector &v2);
float    dot(const glVector &v1, const glVector &v2);

#endif // !defined(AFX_GLVECTOR_H__F526A5CF_89B5_4F20_8F2C_517D83879D35__INCLUDED_)
