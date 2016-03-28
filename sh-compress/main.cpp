#include <iostream>
#include <exception>
#include <vector>

#include "image.h"
#include "montecarlo.h"
#include "rectsampler.h"
#include "sphericalharmonic.h"

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

int main(int argc, char **argv)
{
    // First, initialize all of the external libraries
    initThirdParty(&argc, &argv);

    Image imgIn;
    int nBands = 1;

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
                default:
                    cnt++;
                    break;
            }

            cnt++;
        }
    }

    char strOutName[256] = {};
    char strRecName[256] = {};

    sprintf(strOutName, "copy.png");
    sprintf(strRecName, "rec-l%d.png", nBands);

    std::cout << "SH L: " << nBands << endl;

    try {
        imgIn.LoadFromFile("D:\\Serious\\Doctorado\\code\\bin64\\test.jpg");

        Image imgOut(imgIn.Width, imgIn.Height, imgIn.Format, imgIn.bpp);
        Image imgRec(imgIn.Width, imgIn.Height, imgIn.Format, imgIn.bpp);

        RectSampler rs(imgRec.Width, imgRec.Height);
        SphericalHarmonic sh(&rs, nBands);

        unsigned char *out = imgOut.getPixelData();
        unsigned char *rec = imgRec.getPixelData();
        unsigned char *in = imgIn.getPixelData();

        unsigned int i, j, c;
        int pxStep = imgOut.bpp / 8;
        std::vector<Vector3> colCom;
        std::vector<int> imgC;
        Vector3 colRec;

        imgC.resize(rs.numSamples);
        colCom.resize(sh.numBands2);

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

            for (j = 0; j < sh.numBands2; j++)
            {
                colCom[j].r += colRec.r * sh.Coefficient[i][j];
                colCom[j].g += colRec.g * sh.Coefficient[i][j];
                colCom[j].b += colRec.b * sh.Coefficient[i][j];
            }
        }

        // Scale the function basis
        float scale = (float)(4.0f*M_PI) / (float)rs.numSamples;
        for (j = 0; j < sh.numBands2; j++)
        {
            colCom[j].r *= scale;
            colCom[j].g *= scale;
            colCom[j].b *= scale;
        }

        // Reconstruct the image
        for (i = 0; i < rs.numSamples; i++)
        {
            colRec = Vector3();

            for (j = 0; j < sh.numBands2; j++)
            {
                colRec.r += colCom[j].r * sh.Coefficient[i][j];
                colRec.g += colCom[j].g * sh.Coefficient[i][j];
                colRec.b += colCom[j].b * sh.Coefficient[i][j];
            }

            c = imgC[i];

            rec[c + 0] = (unsigned char)(colRec.r);
            rec[c + 1] = (unsigned char)(colRec.g);
            rec[c + 2] = (unsigned char)(colRec.b);
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
