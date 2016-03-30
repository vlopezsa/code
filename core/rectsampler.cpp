#include "graphics.h"
#include "rectsampler.h"

#define _USE_MATH_DEFINES
#include <math.h>

void RectSampler::calculateSamples()
{
    Vector2 cell, uSphere;
    Vector3 uCartesian;
    int sCnt; // sampler counter

    sCnt = 0;

    // Calculate Sample Position inside the unit sphere
    for (int i = 0; i < this->Height; i++)
        for (int j = 0; j < this->Width; j++)
        {
            cell.x = (float)j / (float)(this->Width);
            cell.y = (float)i / (float)(this->Height);

            uSphere.theta = 2.0f * (float)acos(sqrt(1.0f - cell.x));
            uSphere.phi   = 2.0f * (float)M_PI * cell.y;

            uCartesian.x = (float)(sin(uSphere.theta) * cos(uSphere.phi));
            uCartesian.y = (float)(sin(uSphere.theta) * sin(uSphere.phi));
            uCartesian.z = (float)(cos(uSphere.theta));

            _Samples[sCnt].Square.x = j;
            _Samples[sCnt].Square.y = i;

            _Samples[sCnt].Spherical.theta = uSphere.theta;
            _Samples[sCnt].Spherical.phi   = uSphere.phi;

            _Samples[sCnt].Cartesian.x = uCartesian.x;
            _Samples[sCnt].Cartesian.y = uCartesian.y;
            _Samples[sCnt].Cartesian.z = uCartesian.z;

            sCnt++;
        }
}

RectSampler::RectSampler(unsigned int Width, unsigned int Height)
    : Sampler(Width * Height),
      Width(_Width),
      Height(_Height)
{
    this->_Width = Width;
    this->_Height = Height;

    calculateSamples();
}


RectSampler::~RectSampler()
{
}
