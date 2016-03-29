#include <iostream>

#include "graphics.h"
#include "thirdparty.h"

int main(int argc, char **argv)
{
    initThirdParty(&argc, &argv);

    endThirdParty();

    return 0;
}