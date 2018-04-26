#include <iostream>
#include <iomanip>

#include <windows.h>

#include "osutil.h"

using namespace std;

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
    of.nMaxFile = (DWORD)maxLength;
    of.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES;

    if (GetOpenFileName(&of)) {
        size_t c;
        wcstombs_s(&c, outFileName, maxLength, filename, maxLength);
        res = true;
    }

    delete[] filename;

    return res;
}

void osDisplaySceneInfo(Scene *s, bool extended)
{
    int tabSize = 4;

    cout << "Scene: " << s->strName << endl;
    cout << setw(tabSize) << " ";
    cout << "Meshes: " << s->numMeshes() << endl;

    if (extended)
    {
        for (uint32_t i = 0; i < s->numMeshes(); i++)
        {
            cout << setw(tabSize*2) << " ";
            cout << "\tMesh " << i + 1 << ":" << endl;

            cout << setw(tabSize * 3) << " ";
            cout << "\t\tVertices : " << s->mesh[i].numVertices() << endl;

            cout << setw(tabSize * 3) << " ";
            cout << "\t\tFaces : " << s->mesh[i].numIndices() << endl;

            cout << setw(tabSize * 3) << " ";
            cout << "\t\tTriangles : " << s->mesh[i].numTriangles() << endl;
        }
    }

    cout << setw(tabSize) << " ";
    cout << "Materials: " << s->numMaterials() << endl;

    if (extended)
    {
        for (uint32_t i = 0; i < s->numMaterials(); i++)
        {
            cout << setw(tabSize * 2) << " ";
            cout << "\tMaterial " << i + 1 << ":" << endl;

            cout << setw(tabSize * 3) << " ";
            cout << "\t\tName: " << s->material[i].strName << endl;

            cout << setw(tabSize * 3) << " ";
            cout << "\t\tDiffuse Textures: " << s->material[i].getNumTexDiffuse() << endl;

            cout << setw(tabSize * 3) << " ";
            cout << "\t\tSpecular Textures: " << s->material[i].getNumTexSpecular() << endl;

            cout << setw(tabSize * 3) << " ";
            cout << "\t\tNormal Map Textures: " << s->material[i].getNumTexNormal() << endl;

            cout << setw(tabSize * 3) << " ";
            cout << "\t\tHeight Map Textures: " << s->material[i].getNumTexMask() << endl;
        }
    }

    cout << setw(tabSize) << " ";
    cout << "Triangle Total: " << s->numTriangles() << endl;
    cout << setw(tabSize) << " ";
    cout << "Textures loaded: " << s->texture.getNumTextures() << endl;
}
