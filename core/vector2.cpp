#include <math.h>

#include "Vector2.h"

Vector2::Vector2()
{
    x = y = 0.0f;
}

Vector2::~Vector2()
{

}

Vector2::Vector2(float a, float b, float c)
{
    x = a;
    y = b;
}

Vector2::Vector2(Vector2 &v)
{
    x = v.x;
    y = v.y;
}

float Vector2::length()
{
    return (float)sqrt(x*x + y*y);
}

void Vector2::normalize()
{
    float l = this->length();

    this->x = x / l;
    this->y = y / l;
}

void Vector2::operator *=(float scalar)
{
    this->x *= scalar;
    this->y *= scalar;
}

void Vector2::operator /=(float scalar)
{
    this->x /= scalar;
    this->y /= scalar;
}

void Vector2::operator +=(float scalar)
{
    this->x += scalar;
    this->y += scalar;
}

void Vector2::operator -=(float scalar)
{
    this->x -= scalar;
    this->y -= scalar;
}

void Vector2::operator +=(const Vector2 &v)
{
    this->x += v.x;
    this->y += v.y;
}

void Vector2::operator -=(const Vector2 &v)
{
    this->x -= v.x;
    this->y -= v.y;
}

Vector2 operator *(const Vector2 &v, float scalar)
{
    Vector2 o;
    o.x = v.x * scalar;
    o.y = v.y * scalar;

    return o;
}

Vector2 operator /(const Vector2 &v, float scalar)
{
    Vector2 o;
    o.x = v.x / scalar;
    o.y = v.y / scalar;

    return o;
}

Vector2 operator +(const Vector2 &v, float scalar)
{
    Vector2 o;
    o.x = v.x + scalar;
    o.y = v.y + scalar;

    return o;
}

Vector2 operator -(const Vector2 &v, float scalar)
{
    Vector2 o;
    o.x = v.x - scalar;
    o.y = v.y - scalar;

    return o;
}

Vector2 operator +(const Vector2 &v1, const Vector2 &v2)
{
    Vector2 o;

    o.x = v1.x + v2.x;
    o.y = v1.y + v2.y;

    return o;
}

Vector2 operator -(const Vector2 &v1, const Vector2 &v2)
{
    Vector2 o;

    o.x = v1.x - v2.x;
    o.y = v1.y - v2.y;

    return o;
}
