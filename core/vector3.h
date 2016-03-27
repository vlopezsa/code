#pragma once

class Vector3
{
public:
    Vector3();
    Vector3(float a, float b, float c);
    Vector3(Vector3 &v);
    virtual ~Vector3();

    union
    {
        float d[3];
        struct { float x, y, z; };
        struct { float r, g, b; };
    };

    float length();
    void normalize();

    void operator *=(float scalar);
    void operator /=(float scalar);
    void operator +=(float scalar);
    void operator -=(float scalar);
    void operator +=(const Vector3 &v);
    void operator -=(const Vector3 &v);
};

Vector3 operator *(const Vector3 &v, float scalar);
Vector3 operator /(const Vector3 &v, float scalar);
Vector3 operator +(const Vector3 &v, float scalar);
Vector3 operator -(const Vector3 &v, float scalar);
Vector3 operator +(const Vector3 &v1, const Vector3 &v2);
Vector3 operator -(const Vector3 &v1, const Vector3 &v2);

Vector3 cross(const Vector3 &v1, const Vector3 &v2);
float    dot(const Vector3 &v1, const Vector3 &v2);

