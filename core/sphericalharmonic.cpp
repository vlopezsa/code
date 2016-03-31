#include <exception>
#include <set>

#include "sphericalharmonic.h"

#define _USE_MATH_DEFINES
#include <math.h>

static std::set<SHImplementation> SHImplementation_Set(__SH_IMP_TYPE_DEF);

SphericalHarmonic::SphericalHarmonic(SHImplementation type, Sampler *sampler) :
    shType(_shType),
    Coefficient(_Coefficient),
    numBands(_numBands),
    numBaseCoeff(_numBaseCoeff),
    scaleFactor(_scaleFactor)
{
    if (SHImplementation_Set.find(type) == SHImplementation_Set.end())
    {
        throw std::exception("Invalid Implementation of Spherical Harmonics");
    }

    if (!sampler)
        throw std::exception("Tried to use an invalid sampler");

    this->_shType = type;

    _Sampler = sampler;

    _numBands = 0;

    _numBaseCoeff = 0; //it will calculated later
}

SphericalHarmonic::SphericalHarmonic(SHImplementation type, Sampler * sampler, unsigned int nBands) :
    shType(_shType),
    Coefficient(_Coefficient),
    numBands(_numBands),
    numBaseCoeff(_numBaseCoeff),
    scaleFactor(_scaleFactor)
{
    if (SHImplementation_Set.find(type) == SHImplementation_Set.end())
    {
        throw std::exception("Invalid Implementation of Spherical Harmonics");
    }

    if (!sampler)
        throw std::exception("Tried to use an invalid sampler");

    this->_shType = type;

    _Coefficient.resize(sampler->numSamples);
    _numBands = nBands;
    
    _Sampler = sampler;

    _numBaseCoeff = 0; //it will calculated later
}

SphericalHarmonic::~SphericalHarmonic()
{
}

void SphericalHarmonic::calculateNumberBaseCoeff()
{
    this->_numBaseCoeff = (this->_numBands+1) * (this->_numBands+1);
}

void SphericalHarmonic::setNumBands(unsigned int numBands)
{
    this->_numBands = numBands;
    this->calculateNumberBaseCoeff();
}

bool SphericalHarmonic::scaleFunctionCoeff(std::vector<float>& fCoeff)
{
    if (fCoeff.size() != this->_scaleFactor.size())
        return false;

    for (int i = 0; i < fCoeff.size(); i++)
    {
        fCoeff[i] *= this->_scaleFactor[i];
    }

    return true;
}

bool SphericalHarmonic::scaleFunctionCoeff(std::vector<Vector3>& fCoeff)
{
    if (fCoeff.size() != this->_scaleFactor.size())
        return false;

    for (int i = 0; i < fCoeff.size(); i++)
    {
        fCoeff[i].x *= this->_scaleFactor[i];
        fCoeff[i].y *= this->_scaleFactor[i];
        fCoeff[i].z *= this->_scaleFactor[i];
    }

    return true;
}

float SphericalHarmonic::calculateBasis(unsigned int l, int m, const SamplePoint *s)
{
    return 0.0f;
}

void SphericalHarmonic::calculateCoefficients()
{
    unsigned int i;
    int cnt;

    this->calculateNumberBaseCoeff();

    for (i=0; i < this->_Sampler->numSamples; i++)
    {
        this->_Coefficient[i].resize(this->_numBaseCoeff);

        for (int l = 0; l <= this->_numBands; l++)
            for (int m = -l; m <= l; m++)
            {
                cnt = l*(l + 1) + m;
                /*_Coefficient[i][cnt] = (float)SH(l, m,
                    _Sampler->Samples[i].Spherical.theta,
                    _Sampler->Samples[i].Spherical.phi);*/
                this->_Coefficient[i][cnt] = this->calculateBasis(l, m,
                                                &this->_Sampler->Samples[i]);
            }
    }
}