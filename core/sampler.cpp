#include "sampler.h"

Sampler::Sampler(unsigned int numSamples) :
    numSamples(_numSamples),
    Samples(_Samples)
{
    this->_numSamples = numSamples;
    this->_Samples.resize(numSamples);
}

Sampler::~Sampler()
{
}