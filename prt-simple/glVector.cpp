// glVector.cpp: implementation of the glVector class.
//
//////////////////////////////////////////////////////////////////////

#include "glVector.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

glVector::glVector()
{
	x = y = z = 0.0f;
}

glVector::~glVector()
{

}

glVector::glVector(float a, float b, float c)
{
    x = a;
    y = b;
    z = c;
}

glVector::glVector(glVector &v)
{
    x = v.x;
    y = v.y;
    z = v.z;
}

glVector::glVector(Point3D &p)
{
    x = p.x;
    y = p.y;
    z = p.z;
}

void glVector::operator *=(float scalar)
{
    this->x *= scalar;
    this->y *= scalar;
    this->z *= scalar;
}

void glVector::operator /=(float scalar)
{
    this->x /= scalar;
    this->y /= scalar;
    this->z /= scalar;
}

void glVector::operator +=(float scalar)
{
    this->x += scalar;
    this->y += scalar;
    this->z += scalar;
}

void glVector::operator -=(float scalar)
{
    this->x -= scalar;
    this->y -= scalar;
    this->z -= scalar;
}

void glVector::operator +=(const glVector &v)
{
    this->x += v.x;
    this->y += v.y;
    this->z += v.z;
}

void glVector::operator -=(const glVector &v)
{
    this->x -= v.x;
    this->y -= v.y;
    this->z -= v.z;
}

glVector operator *(const glVector &v, float scalar)
{
    glVector o;
	o.x = v.x * scalar;
    o.y = v.y * scalar;
    o.z = v.z * scalar;

    return o;
}

glVector operator /(const glVector &v, float scalar)
{
    glVector o;
    o.x = v.x / scalar;
    o.y = v.y / scalar;
    o.z = v.z / scalar;

    return o;
}

glVector operator +(const glVector &v, float scalar)
{
    glVector o;
    o.x = v.x + scalar;
    o.y = v.y + scalar;
    o.z = v.z + scalar;

    return o;
}

glVector operator -(const glVector &v, float scalar)
{
    glVector o;
    o.x = v.x - scalar;
    o.y = v.y - scalar;
    o.z = v.z - scalar;

    return o;
}

glVector operator +(const glVector &v1, const glVector &v2)
{
    glVector o;

    o.x = v1.x + v2.x;
    o.y = v1.y + v2.y;
    o.z = v1.z + v2.z;

    return o;
}

glVector operator -(const glVector &v1, const glVector &v2)
{
    glVector o;

    o.x = v1.x - v2.x;
    o.y = v1.y - v2.y;
    o.z = v1.z - v2.z;

    return o;
}

glVector cross(const glVector &v1, const glVector &v2) 
{
    glVector o;

    o.x = v1.y * v2.z - v1.z * v2.y;
    o.y = v1.z * v2.x - v1.x * v2.z;
    o.z = v1.x * v2.y - v1.y * v2.x;

    return o;
}

float dot(const glVector &v1, const glVector &v2)
{
    return	
        v1.x * v2.x +
        v1.y * v2.y +
        v1.z * v2.z;
}