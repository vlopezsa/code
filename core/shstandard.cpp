#include "shstandard.h"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>

SHStandard::SHStandard(Sampler * s)
    : SphericalHarmonic(SHImplementation::SH_Geomerics, s)
{
}

SHStandard::SHStandard(Sampler * sampler, uint32_t numBands)
    : SphericalHarmonic(SHImplementation::SH_Standard, sampler, numBands)
{
}

SHStandard::~SHStandard()
{
}

void SHStandard::calculateScaleFactors()
{
    this->_scaleFactor.assign(
        this->_numBaseCoeff,
        (float)(4.0f * M_PI) / (float)this->_Sampler->numSamples);
}

void SHStandard::calculateNumberBaseCoeff()
{
    this->_numBaseCoeff = (this->numBands + 1) * (this->numBands + 1);
}

float sqrt_2 = (float)sqrt(2);
/*
*/
double P(int l, int m, float x)
{
    float pmm = 1.0;

    if (m>0) {
        float somx2 = (float)sqrt((1.0 - x)*(1.0 + x));
        float fact = 1.0;
        for (int i = 1; i <= m; i++) {
            pmm *= (-fact) * somx2;
            fact += 2.0;
        }
    }
    if (l == m) return pmm;
    float pmmp1 = x * (2.0f*m + 1.0f) * pmm;
    if (l == m + 1) return pmmp1;
    float pll = 0.0;
    for (int ll = m + 2; ll <= l; ++ll) {
        pll = (float)((2.0*ll - 1.0)*x*pmmp1 - (ll + m - 1.0)*pmm) / (ll - m);
        pmm = pmmp1;
        pmmp1 = pll;
    }
    return pll;
}

// K calculation that allows to work with higher order before
// overflowing the data types
// Obtain from: 
//     P.-P. Sloan, “Efficient Spherical Harmonic Evaluation,”
//     Journal of Computer Graphics Techniques (JCGT), vol. 2,
//     no. 2, pp. 84–83, Sep. 2013.

double K(uint32_t l, int m) {
    double uVal = 1;// must be double

    for (uint32_t k = l + m; k > (l - m); k--)
        uVal *= k;

    return sqrt((2.0 * l + 1.0) / (4 * M_PI * uVal));
}

double __inline SH(int l, int m, float theta, float phi)
{
    double res = 0.0f;

    if (m == 0)
        res = K(l, 0) * P(l, 0, cos(theta));
    else if (m < 0)
        res = sqrt_2*K(l, -m)*sin(-m*phi)*P(l, -m, cos(theta));
    else
        res = sqrt_2*K(l, m)*cos(m*phi)*P(l, m, cos(theta));

    return res;
}

float SHStandard::calculateBasis(uint32_t l, int m, const SamplePoint * s)
{
    return (float)SH(l, m, s->Spherical.theta, s->Spherical.phi);
}