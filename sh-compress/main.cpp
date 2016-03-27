#include <iostream>
#include <exception>

#include "image.h"

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

    try {
        imgIn.LoadFromFile("D:\\Serious\\Doctorado\\code\\bin64\\test.jpg");

        Image imgOut(imgIn.Width, imgIn.Height, imgIn.Format, imgIn.bpp);

        imgOut.SaveToFile("output.jpg");
    }
    catch (std::exception &e)
    {
        cout << e.what() << endl;
    }

    endThirdParty();

    return 0;
}