#include <math.h>

#include "vector4.h"
#include "vector3.h"

Vector3::Vector3()
{
    x = y = z = 0.0f;
}

Vector3::~Vector3()
{

}

Vector3::Vector3(float a)
{
    x = a;
    y = a;
    z = a;
}

Vector3::Vector3(float a, float b, float c)
{
    x = a;
    y = b;
    z = c;
}

Vector3::Vector3(const Vector3 &v)
{
    x = v.x;
    y = v.y;
    z = v.z;
}

void Vector3::operator *=(float scalar)
{
    this->x *= scalar;
    this->y *= scalar;
    this->z *= scalar;
}

void Vector3::operator /=(float scalar)
{
    this->x /= scalar;
    this->y /= scalar;
    this->z /= scalar;
}

void Vector3::operator +=(float scalar)
{
    this->x += scalar;
    this->y += scalar;
    this->z += scalar;
}

void Vector3::operator -=(float scalar)
{
    this->x -= scalar;
    this->y -= scalar;
    this->z -= scalar;
}

void Vector3::operator +=(const Vector3 &v)
{
    this->x += v.x;
    this->y += v.y;
    this->z += v.z;
}

void Vector3::operator -=(const Vector3 &v)
{
    this->x -= v.x;
    this->y -= v.y;
    this->z -= v.z;
}

void Vector3::operator *=(const Vector3 &v)
{
    this->x *= v.x;
    this->y *= v.y;
    this->z *= v.z;
}

float Vector3::length()
{
    return (float)sqrt(x*x + y*y + z*z);
}

void Vector3::normalize()
{
    float l = this->length();

    this->x = x / l;
    this->y = y / l;
    this->z = z / l;
}

Vector3 operator *(const Vector3 &v, float scalar)
{
    Vector3 o;
    o.x = v.x * scalar;
    o.y = v.y * scalar;
    o.z = v.z * scalar;

    return o;
}

Vector3 operator /(const Vector3 &v, float scalar)
{
    Vector3 o;
    o.x = v.x / scalar;
    o.y = v.y / scalar;
    o.z = v.z / scalar;

    return o;
}

Vector3 operator +(const Vector3 &v, float scalar)
{
    Vector3 o;
    o.x = v.x + scalar;
    o.y = v.y + scalar;
    o.z = v.z + scalar;

    return o;
}

Vector3 operator -(const Vector3 &v, float scalar)
{
    Vector3 o;
    o.x = v.x - scalar;
    o.y = v.y - scalar;
    o.z = v.z - scalar;

    return o;
}

Vector3 operator +(const Vector3 &v1, const Vector3 &v2)
{
    Vector3 o;

    o.x = v1.x + v2.x;
    o.y = v1.y + v2.y;
    o.z = v1.z + v2.z;

    return o;
}

Vector3 operator -(const Vector3 &v1, const Vector3 &v2)
{
    Vector3 o;

    o.x = v1.x - v2.x;
    o.y = v1.y - v2.y;
    o.z = v1.z - v2.z;

    return o;
}

Vector3 operator *(const Vector3 &v1, const Vector3 &v2)
{
    Vector3 o;

    o.x = v1.x * v2.x;
    o.y = v1.y * v2.y;
    o.z = v1.z * v2.z;

    return o;
}

Vector3 cross(const Vector3 &v1, const Vector3 &v2)
{
    Vector3 o;

    o.x = v1.y * v2.z - v1.z * v2.y;
    o.y = v1.z * v2.x - v1.x * v2.z;
    o.z = v1.x * v2.y - v1.y * v2.x;

    return o;
}

float dot(const Vector3 &v1, const Vector3 &v2)
{
    return
        v1.x * v2.x +
        v1.y * v2.y +
        v1.z * v2.z;
}