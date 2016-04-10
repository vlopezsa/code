#pragma once

#include "graphics.h"

class Camera
{
public:
    float m_MaxPitchRate;
    float m_MaxHeadingRate;
    float m_HeadingDegrees;
    float m_PitchDegrees;
    float m_MaxForwardVelocity;
    float m_ForwardVelocity;
    Quaternion m_qHeading;
    Quaternion m_qPitch;
    
    Vector3   m_Position;
    Vector3   m_Direction;
    Vector3   m_Right;
    Vector3   m_Up;

public:
    Camera();
    virtual ~Camera();

    void StoreValues();

    void ChangeVelocity(float vel);
    void ChangeHeading(float degrees);
    void ChangePitch(float degrees);
    void SetPerspective(void);
    void SetPosition(Vector3 &Pos);
};
