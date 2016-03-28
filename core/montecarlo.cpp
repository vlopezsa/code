#include <time.h>
#include "graphics.h"
#include "montecarlo.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define real_rand() ((float)rand()/RAND_MAX)

void MonteCarlo::calculateSamples()
{
    Vector2 cell, uSphere;
    Vector3 uCartesian;
    int sN;
    int sCnt; // sampler counter

    sN = (int)sqrt(this->numSamples);

    srand((unsigned int)time(NULL));

    sCnt = 0;

    // Calculate Sample Position inside the unit sphere
    for (int i = 0; i < sN; i++)
        for (int j = 0; j < sN; j++)
        {
            cell.x = ((float)i + real_rand()) / (float)sN;
            cell.y = ((float)j + real_rand()) / (float)sN;

            uSphere.theta = 2 * acos(sqrt(1.0f - cell.x));
            uSphere.phi = 2 * (float)M_PI * cell.y;

            uCartesian.x = sin(uSphere.theta) * cos(uSphere.phi);
            uCartesian.y = sin(uSphere.theta) * sin(uSphere.phi);
            uCartesian.z = cos(uSphere.theta);

            _Samples[sCnt].Square.x = j;
            _Samples[sCnt].Square.y = i;

            _Samples[sCnt].Spherical.theta = uSphere.theta;
            _Samples[sCnt].Spherical.phi = uSphere.phi;

            _Samples[sCnt].Cartesian.x = uCartesian.x;
            _Samples[sCnt].Cartesian.y = uCartesian.y;
            _Samples[sCnt].Cartesian.z = uCartesian.z;

            sCnt++;
        }
}

MonteCarlo::MonteCarlo(unsigned int numSamples)
    : Sampler(numSamples)
{
    calculateSamples();
}


MonteCarlo::~MonteCarlo()
{
}
