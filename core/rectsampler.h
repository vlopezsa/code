#pragma once
#include "sampler.h"
class RectSampler :
    public Sampler
{
private:
    unsigned int _Width;
    unsigned int _Height;

protected:
    virtual void calculateSamples();

public:
    const unsigned int &Width;
    const unsigned int &Height;

public:
    RectSampler(unsigned int Width, unsigned int Height);
    ~RectSampler();
};