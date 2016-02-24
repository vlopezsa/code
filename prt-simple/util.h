#ifndef UTIL_H
#define UTIL_H

struct Point2D
{
	float x,y;
};

struct Point3D {
	float x,y,z;
};

struct Triangle {
	int a, b, c;
};

typedef struct {
    float r, g, b;
} tPixel3;

typedef struct {
    float r, g, b, a;
} tPixel4;

#define MAX_TEXTURES 8
#define CUBE_TEXTURE_NUM 6

#define FBO_TEXTURE MAX_TEXTURES-1

//declared in main.cpp
void UpdateEyePositionFromMouse();
void SetupLighting();

#endif