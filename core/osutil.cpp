#include <windows.h>

#include "osutil.h"

bool osOpenDlg(char *outFileName, size_t maxLength)
{
    wchar_t *filename;
    OPENFILENAME of;
    bool res = false;

    filename = new wchar_t[maxLength];

    memset(&of, 0, sizeof(of));
    memset(filename, 0, sizeof(wchar_t) * maxLength);

    of.lStructSize = sizeof(OPENFILENAME);
    of.hwndOwner = NULL;
    of.hInstance = GetModuleHandle(NULL);
    of.lpstrFilter = L"*.*\0\0";
    of.lpstrFile = filename;
    of.nMaxFile = maxLength;
    of.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES;

    if (GetOpenFileName(&of)) {
        wcstombs(outFileName, filename, maxLength);
        res = true;
    }

    delete[] filename;

    return res;
}