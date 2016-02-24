#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <windows.h>

extern ULONG_PTR gdiplusToken;
class ImageLoader
{
public:
	static void Init();
	static void Close();
	static unsigned char *LoadFile(wchar_t *sFileName, int *out_Width, int *out_Height);
};

#endif