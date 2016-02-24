#include "ImageLoader.h"

#include <gdiplus.h>
#include <stdio.h>

ULONG_PTR gdiplusToken;

void ImageLoader::Init()
{
   Gdiplus::GdiplusStartupInput gdiplusStartupInput;
   
   // Initialize GDI+.
   Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

void ImageLoader::Close()
{
	Gdiplus::GdiplusShutdown(gdiplusToken);
}

unsigned char *ImageLoader::LoadFile(wchar_t *sFileName, int *out_Width, int *out_Height)
{
	unsigned char *Data = NULL;
	// Load the file
    Gdiplus::Image *oImage = new Gdiplus::Image(sFileName);
    
    if (oImage->GetLastStatus() != Gdiplus::Status::Ok)
        return NULL;

	Gdiplus::PixelFormat oPixelFormat = oImage->GetPixelFormat();
	
    int Width = oImage->GetWidth();
	int Height = oImage->GetHeight();

	// Create a new bitmap
	Gdiplus::Bitmap *oBitmap = new Gdiplus::Bitmap(Width, Height, PixelFormat24bppRGB);
	Gdiplus::Graphics *oGraphics = new Gdiplus::Graphics(static_cast<Gdiplus::Image *>(oBitmap));
	// Render the image into the new bitmap
	Gdiplus::Rect oRect(0,0, Width, Height);
	oGraphics->DrawImage(oImage, oRect);

    // Lock the image
    Gdiplus::BitmapData LockedBitmapData;    
    oBitmap->LockBits(&oRect, 
					  Gdiplus::ImageLockModeRead, 
					  //PixelFormat32bppARGB, 
					  PixelFormat24bppRGB,
					  &LockedBitmapData);

	// Read the information
	unsigned int Size = 3*Width*Height;
	Data = new unsigned char[Size];
	int x,y, StartOfs = 0, SrcOfs = 0, DestOfs = 0;
	unsigned char *SrcData = (unsigned char *)LockedBitmapData.Scan0;
	//FILE *File = fopen("Output.txt", "wt");
	for (y = 0; y < Height; y++)
	{
		SrcOfs = StartOfs;
		for (x = 0; x < Width; x++)
		{
			unsigned char B = SrcData[SrcOfs]; SrcOfs++;
			unsigned char G = SrcData[SrcOfs]; SrcOfs++;
			unsigned char R = SrcData[SrcOfs]; SrcOfs++;
			//SrcOfs++;

			if (DestOfs >= Size)
			{
				printf("Exceeded destination bitmap size!\n");
			}
			Data[DestOfs] = R; DestOfs++;
			Data[DestOfs] = G; DestOfs++;
			Data[DestOfs] = B; DestOfs++;
			//fprintf(File, "%c", (int )(24.0f * R/255.0f) + 'a');
		}
		//fprintf(File, "\n");
		StartOfs += LockedBitmapData.Stride;
	}
	//fclose(File);

	// Unlock the image
    oBitmap->UnlockBits(&LockedBitmapData);

	// Tidy up
    delete oImage;
	delete oBitmap;
	delete oGraphics;

	*out_Width = Width;
	*out_Height = Height;	
	return Data;
}

