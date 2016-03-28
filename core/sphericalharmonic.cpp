#include <exception>

#include "sphericalharmonic.h"

#define _USE_MATH_DEFINES
#include <math.h>

SphericalHarmonic::SphericalHarmonic(Sampler * sampler, unsigned int nBands) :
    Coefficient(_Coefficient),
    numBands(_numBands),
    numBands2(_numBands2)
{
    if (!sampler)
        throw std::exception("Tried to use an invalid sampler");

    if(nBands < 1)
        throw std::exception("Invalid number of bands");

    _Coefficient.resize(sampler->numSamples);
    _numBands = nBands;
    _numBands2 = nBands * nBands;

    _Sampler = sampler;

    try
    {
        calculateCoefficients();
    }
    catch (std::exception &e)
    {
        throw;
    }
}

SphericalHarmonic::~SphericalHarmonic()
{
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

/*unsigned long long nfactorial[21]
{
    1ULL, //0!
    1ULL, //1!
    2ULL, //2!
    6ULL, //3!
    24ULL, //4!
    120ULL, //5!
    720ULL, //6!
    5040ULL, //7!
    40320ULL, //8!
    362880ULL, //9!
    3628800ULL, //10!
    39916800ULL, //11!
    479001600ULL, //12!
    6227020800ULL, //13!
    87178291200ULL, //14!
    1307674368000ULL, //15!
    20922789888000ULL, //16!
    355687428096000ULL, //17!
    6402373705728000ULL, //18!
    121645100408832000ULL, //19!
    2432902008176640000ULL, //20!
    // 51090942171709440000ULL, //21! Can't be hold by unsigned long long
};*/


/*unsigned long long factorial(int x)
{
    // The maximun factorial of a number
    // that can be computed is 20
    if (x > 20)
        x = 20;

    return nfactorial[x];
}

double K(int l, int m)
{
    double k1 = (double)((2 * l + 1)*factorial(l - m));
    double k2 = (double)factorial(l + m);
    double k3 = (double)(4.0f*M_PI*k2);
    double temp = k1 / k3;
    return sqrt(temp);
}*/

// K calculation that allows to work with higher order before
// overflowing the data types
// Obtain from: 
//     P.-P. Sloan, “Efficient Spherical Harmonic Evaluation,”
//     Journal of Computer Graphics Techniques (JCGT), vol. 2,
//     no. 2, pp. 84–83, Sep. 2013.

double K(int l, int m) {
    double uVal = 1;// must be double

    for (unsigned int k = l + m; k > (l - m); k--)
        uVal *= k;

    return sqrt((2.0 * l + 1.0) / (4 * M_PI * uVal));
}

double SH(int l, int m, float theta, float phi)
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

void SphericalHarmonic::calculateCoefficients()
{
    unsigned int i;
    int cnt;

    for (i=0; i < _Sampler->numSamples; i++)
    {
        _Coefficient[i].resize(_numBands2);

        for (int l = 0; l < (int)_numBands; l++)
            for (int m = -l; m <= l; m++)
            {
                cnt = l*(l + 1) + m;
                _Coefficient[i][cnt] = (float)SH(l, m,
                    _Sampler->Samples[i].Spherical.theta,
                    _Sampler->Samples[i].Spherical.phi);
            }
    }
}