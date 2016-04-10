#include <iostream>
#include <exception>
#include <vector>

#include "image.h"
#include "montecarlo.h"
#include "rectsampler.h"
#include "shstandard.h"
#include "shgeomerics.h"

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

void initThirdParty(int *argc, char ***argv)
{
    FreeImage_Initialise();
}

void endThirdParty()
{
    FreeImage_DeInitialise();
}

float __inline clamp(float value, const float min, const float max)
{
    if (value < min)
        return min;

    if (value > max)
        return max;

    return value;
}

int main(int argc, char **argv)
{
    // First, initialize all of the external libraries
    initThirdParty(&argc, &argv);

    Image imgIn;
    int nBands = 2;

    char strOutName[256] = {};
    char strRecName[256] = {};
    char strInName[256] = {};

    sprintf(strInName, "D:\\Serious\\Doctorado\\code\\bin64\\simple.png");

    if (argc > 1)
    {
        int cnt = 1;
        while (cnt < argc)
        {
            switch (argv[cnt][0])
            {
                case 'l': case 'L':
                    nBands = atoi(argv[++cnt]);
                    break;
                case 'f': case 'F':
                    strncpy(strInName, argv[++cnt], sizeof(strInName));
                    break;
                default:
                    cnt++;
                    break;
            }

            cnt++;
        }
    }

    sprintf(strOutName, "copy.png");
    sprintf(strRecName, "rec-l%d.png", nBands);

    std::cout << "SH L: " << nBands << endl;

    try {
        imgIn.LoadFromFile(strInName);

        Image imgOut(imgIn.Width, imgIn.Height, imgIn.Format, imgIn.bpp);
        Image imgRec(imgIn.Width, imgIn.Height, imgIn.Format, imgIn.bpp);

        RectSampler rs(imgRec.Width, imgRec.Height);
        //SHStandard sh(&rs, nBands);
        SHGeomerics sh(&rs, nBands);

        sh.calculateCoefficients();

        uint8_t *out = imgOut.getPixelData();
        uint8_t *rec = imgRec.getPixelData();
        uint8_t *in = imgIn.getPixelData();

        unsigned int i, j, c;
        int pxStep = imgOut.bpp / 8;
        std::vector<Vector3> colCom;
        std::vector<int> imgC;
        Vector3 colRec;

        imgC.resize(rs.numSamples);
        colCom.resize(sh.numBaseCoeff);

        // Getting positions inside our image buffer
        // for each point on the sphere
        for (i = 0; i < rs.numSamples; i++)
        {
            c = (rs.Samples[i].Square.y * imgOut.Pitch) + (rs.Samples[i].Square.x * pxStep);

            imgC[i] = c;
        }

        // Calculate sh function basis
        for (i = 0; i < rs.numSamples; i++)
        {
            c = imgC[i];

            out[c + 0] = in[c + 0];
            out[c + 1] = in[c + 1];
            out[c + 2] = in[c + 2];

            colRec.r = (float)in[c + 0];
            colRec.g = (float)in[c + 1];
            colRec.b = (float)in[c + 2];

            for (j = 0; j < sh.numBaseCoeff; j++)
            {
                colCom[j].r += colRec.r * sh.Coefficient[i][j];
                colCom[j].g += colRec.g * sh.Coefficient[i][j];
                colCom[j].b += colRec.b * sh.Coefficient[i][j];
            }
        }

        // Scale the function basis
        sh.calculateScaleFactors();        

        // Reconstruct the image
        if (sh.shType == SHImplementation::SH_Geomerics)
        {
            for (i = 0; i < rs.numSamples; i++)
            {
                colRec = Vector3();

                for (j = 0; j < sh.numBaseCoeff; j++)
                {
                    colRec.r += colCom[j].r * sh.Coefficient[i][j] * sh.scaleFactor[j];
                    colRec.g += colCom[j].g * sh.Coefficient[i][j] * sh.scaleFactor[j];
                    colRec.b += colCom[j].b * sh.Coefficient[i][j] * sh.scaleFactor[j];
                }

                c = imgC[i];

                rec[c + 0] = (uint8_t)(clamp(colRec.r, 0.0f, 255.0f));
                rec[c + 1] = (uint8_t)(clamp(colRec.g, 0.0f, 255.0f));
                rec[c + 2] = (uint8_t)(clamp(colRec.b, 0.0f, 255.0f));
            }
        }
        else
        {
            sh.scaleFunctionCoeff(colCom);

            for (i = 0; i < rs.numSamples; i++)
            {
                colRec = Vector3();

                for (j = 0; j < sh.numBaseCoeff; j++)
                {
                    colRec.r += colCom[j].r * sh.Coefficient[i][j];
                    colRec.g += colCom[j].g * sh.Coefficient[i][j];
                    colRec.b += colCom[j].b * sh.Coefficient[i][j];
                }

                c = imgC[i];

                rec[c + 0] = (uint8_t)(clamp(colRec.r, 0.0f, 255.0f));
                rec[c + 1] = (uint8_t)(clamp(colRec.g, 0.0f, 255.0f));
                rec[c + 2] = (uint8_t)(clamp(colRec.b, 0.0f, 255.0f));
            }
        }        

        // Save results
        imgOut.SaveToFile(strOutName);
        imgRec.SaveToFile(strRecName);
    }
    catch (std::exception &e)
    {
        cout << e.what() << endl;
    }

    endThirdParty();

    return 0;
}
