#pragma once

class Quaternion
{
public:
    Quaternion operator *(Quaternion q);
    void CreateMatrix(float *pMatrix);
    void CreateFromAxisAngle(float x, float y, float z, float degrees);
    Quaternion();
    virtual ~Quaternion();

private:
    float m_w;
    float m_z;
    float m_y;
    float m_x;
};
