#include <vector>
#include "SHGeomerics.h"

#define _USE_MATH_DEFINES
#include <math.h>

float __geomeric_f[] = {
    1.0f,
    3.0f, 3.0f, 3.0f,
    7.5f, 7.5f, 7.5f, 7.5f, 7.5f
};

SHGeomerics::SHGeomerics(Sampler * sampler)
    : SphericalHarmonic(SHImplementation::SH_Geomerics, sampler)    
{
    this->_scaleFactor.assign(__geomeric_f,
        __geomeric_f + (sizeof(__geomeric_f) / sizeof(__geomeric_f[0])));

}

SHGeomerics::SHGeomerics(Sampler * sampler, unsigned int numBands)
    : SphericalHarmonic(SHImplementation::SH_Geomerics, sampler, numBands)
{
    // This implementations has a limit of 2
    if (numBands > 2)
        this->setNumBands(2);

    this->_scaleFactor.assign(__geomeric_f,
        __geomeric_f + (sizeof(__geomeric_f) / sizeof(__geomeric_f[0])));
}

SHGeomerics::~SHGeomerics()
{
}

void SHGeomerics::calculateScaleFactors()
{
    for (int i = 0; i < this->_scaleFactor.size(); i++)
        this->_scaleFactor[i] = this->_scaleFactor[i] / (float)this->_Sampler->numSamples;
}

void SHGeomerics::calculateNumberBaseCoeff()
{
    this->_numBaseCoeff = (this->numBands + 1) * (this->numBands + 1);
}

float SHGeomerics::calculateBasis(unsigned int l, int m, const SamplePoint * s)
{
    // In the explanation only up to L2 is defined
    // For testing purposes, i'll leave it up to that level
    if (l > 2)
        return 0.0f;

    if(l == 0)
        return 1.0f;

    if (l == 1)
    {
        switch (m)
        {
        case -1: return s->Cartesian.x;
        case  0: return s->Cartesian.y;
        case  1: return s->Cartesian.z;
        }
    }

    if (l == 2)
    {
        float x = s->Cartesian.x;
        float y = s->Cartesian.z;
        float z = s->Cartesian.y;

        switch (m)
        {
        case -2: return y*x;
        case -1: return y*z;
        case  0: return (3.0f*z*z) - 1.0f; 
        case  1: return x*z;
        case  2: return (x * x) - (y * y);
        }
    }

    return 0.0f;
}