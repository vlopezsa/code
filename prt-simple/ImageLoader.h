#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

class ImageLoader
{
public:
	static void Init();
	static void Close();
	static uint8_t *LoadFile(char *sFileName, int *out_Width, int *out_Height);
};

#endif