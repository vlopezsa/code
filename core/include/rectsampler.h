#pragma once
#include "sampler.h"
class RectSampler :
    public Sampler
{
private:
    uint32_t _Width;
    uint32_t _Height;

protected:
    virtual void calculateSamples();

public:
    const uint32_t &Width;
    const uint32_t &Height;

public:
    RectSampler(uint32_t Width, uint32_t Height);
    ~RectSampler();
};