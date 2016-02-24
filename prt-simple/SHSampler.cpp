#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "LightModel.h"
#include "SHSampler.h"

#pragma warning(disable:4244)

#ifndef PI
#define PI			3.14159265358979323846
#endif

CSHSampler::CSHSampler()
{
    nBands = 0;
    sphereSample = NULL;
}

CSHSampler::~CSHSampler()
{
    if (sphereSample)
        delete[] sphereSample;

    sphereSample = NULL;
}

void CSHSampler::deleteShCoeff()
{
    if (sphereSample == NULL)
        return;

    for (int i = 0; i < nSamples; i++)
    {
        if (sphereSample[i].sh != NULL)
        {
            delete[] sphereSample[i].sh;
            sphereSample[i].sh = NULL;
        }
    }
}

void CSHSampler::setNumberSamples(int samples, int bands)
{
    if (samples == nSamples)
        return;

    nSamples = (samples <= 0) ? 1 : samples;
    
    nBands = (bands < 0) ? 0 : bands;

    if (sphereSample)
    {
        deleteShCoeff();
        delete[] sphereSample;
    }

    sphereSample = new tSphereSample[samples];

    memset(sphereSample, 0, sizeof(tSphereSample) * samples);
}

float sqrt_2 = (float)sqrt(2);

int factorial(int x)
{
    int res = 1;
    int i;

    if (x < 1)
        return 1;

    for (i = 1; i <= x; i++)
        res = res*i;

    return res;
}

float K(int l, int m)
{
    double temp = ((2.0*l + 1.0)*factorial(l - m)) / (4.0*PI*factorial(l + m));
    return (float)sqrt(temp);
}

float P(int l, int m, float x)
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

float SH(int l, int m, float theta, float phi)
{
    float res = 0.0f;

    if (m == 0)
        res = K(l, 0) * P(l, 0, cos(theta));
    else if (m < 0)
        res = sqrt_2*K(l, m)*sin(-m*phi)*P(l, -m, cos(theta));
    else
        res = sqrt_2*K(l, m)*cos(m*phi)*P(l, m, cos(theta));

    return res;
}

void CSHSampler::calculateSH(float **sh, Point2D *uSphere)
{
    int shCnt = 0;

    if (*sh != NULL)
        delete[] * sh;

    *sh = new float[nBands * nBands + 1];
 
    for (int l = 0; l < nBands; l++)
        for (int m = -l; m <= l; m++)
        {
            (*sh)[shCnt] = SH(l, m, uSphere->x, uSphere->y);
        }

}

#define real_rand() (float)((rand()%10000)/10000.0f)
void CSHSampler::calculateSamples()
{
    Point2D cell, uSphere;
    Point3D uCartesian;
    int sN;
    int sCnt; // sampler counter

    sN = (int)sqrt(nSamples);

    srand(time(NULL));

    sCnt = 0;

    // Calculate Sample Position inside the unit sphere
    for (int i = 0; i < sN; i++)
        for (int j = 0; j < sN; j++)
        {
            cell.x = (i + real_rand()) / sN;
            cell.y = (j + real_rand()) / sN;

            uSphere.x = 2 * acos(sqrt(1.0f - cell.x));
            uSphere.y = 2 * PI * cell.y;

            uCartesian.x = sin(uSphere.x) * cos(uSphere.y);
            uCartesian.y = sin(uSphere.x) * sin(uSphere.y);
            uCartesian.z = cos(uSphere.x);

            sphereSample[sCnt].sphr.x = uSphere.x;
            sphereSample[sCnt].sphr.y = uSphere.y;

            sphereSample[sCnt].cart.x = uCartesian.x;
            sphereSample[sCnt].cart.y = uCartesian.y;
            sphereSample[sCnt].cart.z = uCartesian.z;

            sphereSample[sCnt].sh = NULL;

            calculateSH(&sphereSample[sCnt].sh, &uSphere);

            sCnt++;
        }
}
