#pragma once

class Vector2
{
public:
    Vector2();
    Vector2(float a, float b);
    Vector2(Vector2 &v);
    virtual ~Vector2();

    float length();
    void normalize();

    void operator *=(float scalar);
    void operator /=(float scalar);
    void operator +=(float scalar);
    void operator -=(float scalar);
    void operator +=(const Vector2 &v);
    void operator -=(const Vector2 &v);

    union
    {
        float d[2];
        struct { float x, y; };
        struct { float u, v; };
        struct { float phi, theta; };
    };
};

Vector2 operator *(const Vector2 &v, float scalar);
Vector2 operator /(const Vector2 &v, float scalar);
Vector2 operator +(const Vector2 &v, float scalar);
Vector2 operator -(const Vector2 &v, float scalar);
Vector2 operator +(const Vector2 &v1, const Vector2 &v2);
Vector2 operator -(const Vector2 &v1, const Vector2 &v2);
