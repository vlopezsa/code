#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

typedef struct {
	char file[1024];
	unsigned char *data;

	int  width;
	int  height;
}tTexture;

typedef struct {
	Point3D p;   // x, y, z
	Point3D n;   // normal
	Point2D t;   // tex Coord
	Point3D tan; // tangent direction
	int     c;   // color
} tVertex;

typedef struct {
	int			 nIdxTex;
	int			 nIdxNmTex;
	int			 nVertex;
	int			 nIndex;

	unsigned int *index;
	tVertex		 *vertex;
} tMesh;

typedef struct {
	int			nMesh;
	int			nTex;

	tMesh	    *mesh;
	tTexture	*tex;

    char        file[1024];
} tModel;

int LoadModel(char *strFile, tModel *m);

void ReleaseModel(tModel *m);

#endif
