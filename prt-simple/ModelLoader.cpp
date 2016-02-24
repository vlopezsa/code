#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "ModelLoader.h"
#include "VectorMath.h"
#include "ImageLoader.h"

#define BUF_SIZE 2048  //Line max size

void ReleaseModel(tModel *m) {
	int i;

	if(m->mesh == NULL)
		return;

	for(i=0; i<m->nMesh; i++) {
		if(m->mesh[i].vertex)
			free(m->mesh[i].vertex);
		m->mesh[i].vertex = NULL;
	}

	for(i=0; i<m->nTex; i++) {
		free(m->tex[i].data);
		m->tex[i].data = NULL;
	}

	free(m->mesh);
	m->mesh = NULL;

	free(m->tex);
	m->tex = NULL;
}

int AddTexture(tModel *m, char *file) {
	int i;

	for(i=0; i < m->nTex; i++) {
		if(!strcmp(m->tex[i].file, file))
			break;
	}

	if(i<m->nTex)
		return i;

	m->nTex++;
	m->tex = (tTexture *)realloc(m->tex, sizeof(tTexture) * m->nTex);

	wchar_t wbuf[1024]={0};

	mbstowcs(wbuf, file, strlen(file));
	
	strcpy(m->tex[i].file, file);
	m->tex[i].data = ImageLoader::LoadFile(wbuf, &m->tex[i].width, &m->tex[i].height);

	return i;
}

int AddNormalMap(tModel *m, char *file) {
	int i=-1;

	int l = strlen(file);

	for(i=l-1; i>=0 && file[i]!='.'; i--);

	char buf[256] = {0};

	strncpy(buf, file, i);
	strcat(buf, "_nm");
	strcat(buf, file+i);

	return AddTexture(m, buf);
}

int AddMesh(tModel *m, FILE *fmt) {
	m->mesh = (tMesh *)realloc(m->mesh, sizeof(tMesh) * (m->nMesh+1));
	memset(&m->mesh[m->nMesh], 0, sizeof(tMesh));

	char buf[BUF_SIZE] = {0};
	char *tmp;

	int i,j,k;
	
	fgets(buf, BUF_SIZE, fmt);

	while(!strstr(buf, "</buffer>") && !feof(fmt)) {
		if(tmp=strstr(buf, "<texture name=\"Texture1\"")) {
			/*Got the actual texture path*/
			tmp=strstr(buf, "value=\"");
			tmp+=strlen("value=\"");

			i=0;
			while(tmp[i]!='"') i++;
			tmp[i] = 0;

			m->mesh[m->nMesh].nIdxTex   = AddTexture(m, tmp);
			m->mesh[m->nMesh].nIdxNmTex = AddNormalMap(m, tmp);

		} else if(tmp=strstr(buf, "<vertices")) {
			/*Got the number of vertices to read*/
			tmp=strstr(buf, "vertexCount=");

			tmp+=strlen("vertexCount=")+1;

			i=0;
			while(tmp[i]!='"') i++;
			tmp[i] = 0;

			m->mesh[m->nMesh].nVertex = atoi(tmp);
			
			m->mesh[m->nMesh].vertex = (tVertex *)malloc(sizeof(tVertex) * m->mesh[m->nMesh].nVertex);

			/*Start reading vertices data;*/
			i=0;
			fgets(buf, BUF_SIZE, fmt);
			while(!strstr(buf, "</vertices>") && i<m->mesh[m->nMesh].nVertex) {
				sscanf(buf, "%f %f %f %f %f %f %x %f %f",
					&m->mesh[m->nMesh].vertex[i].p.x, &m->mesh[m->nMesh].vertex[i].p.y, &m->mesh[m->nMesh].vertex[i].p.z,
					&m->mesh[m->nMesh].vertex[i].n.x, &m->mesh[m->nMesh].vertex[i].n.y, &m->mesh[m->nMesh].vertex[i].n.z,
					&m->mesh[m->nMesh].vertex[i].c,
					&m->mesh[m->nMesh].vertex[i].t.x, &m->mesh[m->nMesh].vertex[i].t.y);

				i++;
				fgets(buf, BUF_SIZE, fmt);
			} 

			/*Load Index data*/		
		} else if(tmp=strstr(buf, "<indices")) {
			/*Got the number of vertices to read*/
			tmp=strstr(buf, "indexCount=");
			tmp+=strlen("indexCount=")+1;

			i=0;
			while(tmp[i]!='"') i++;
			tmp[i] = 0;
			
			m->mesh[m->nMesh].nIndex = atoi(tmp);;
			m->mesh[m->nMesh].index  = (unsigned int *)malloc(sizeof(unsigned int) * m->mesh[m->nMesh].nIndex);
			
			/*Start reading index data;*/
			i=0;
			fgets(buf, BUF_SIZE, fmt);
			
			while(!strstr(buf, "</indices>") && i<m->mesh[m->nMesh].nIndex) {
				int l = strlen(buf);
				int k = 0;
				j = 0;

				while(j<l) {
					if(buf[j]>='0' && buf[j]<='9') {
						k = j;
						while(buf[j]>='0' && buf[j]<='9') j++;

						buf[j]=0;
						m->mesh[m->nMesh].index[i] = atoi(buf+k);
						i++;
					}

					j++;
				}

				fgets(buf, BUF_SIZE, fmt);
			}
		}

		fgets(buf, BUF_SIZE, fmt);
	}

	m->nMesh++;

	return m->nMesh-1;
}

void ComputeTangentSpace(tModel *Model)
{
	float Dot;
	float Normal[3];
	float Tangent[3];
	float Binormal[3];

	tMesh Mesh;
	tVertex *Vertices;

	for (int j = 0; j < Model->nMesh; j++)
	{
		Mesh = Model->mesh[j];
		Vertices = Mesh.vertex;

		for (int i = 0; i < Mesh.nVertex; i++)
		{
			Normal[0] = Vertices[i].n.x;
			Normal[1] = Vertices[i].n.y;
			Normal[2] = Vertices[i].n.z;
			Normalize(Normal);

			// Create a tentative "tangent" which points in the positive X direction
			Tangent[0] = 1;
			Tangent[1] = 0;
			Tangent[2] = 0;
			Dot = DotProduct(Normal, Tangent);
			if (Dot > 0.9) // If the normal and the tangent are too parallel, switch the tangent!
			{
				Tangent[0] = 0;
				Tangent[1] = 0;
				Tangent[2] = 1;
			}
			else if (Dot < -0.9)
			{
				Tangent[0] = 0;
				Tangent[1] = 0;
				Tangent[2] = -1;
			}

			// Compute the tentative binormal
			CrossProduct(Binormal, Normal, Tangent);
			
			// The normal and the tangent are not orthogonal, compute the real tangent now
			CrossProduct(Tangent, Binormal, Normal);
			Normalize(Tangent);

			// Copy the tangent back to the model
			Vertices[i].tan.x = Tangent[0];
			Vertices[i].tan.y = Tangent[1];
			Vertices[i].tan.z = Tangent[2];
		}
	}
}

int LoadModel(char *strFile, tModel *m) {
	FILE *fmt;
	int iSize = 0, i;
	printf("Loading model %s...\n", strFile);

	fmt = fopen(strFile, "r");
	if(!fmt)
		return -1;
	
	/*Initialize model structure*/
	m->nMesh = 0;
	m->mesh  = NULL;

	/*Parse File*/
	char buf[BUF_SIZE]={0};
	
	i = -1;
	while(!feof(fmt)) {
		fgets(buf, BUF_SIZE, fmt);

		if(strstr(buf, "<buffer>")) {
			i = AddMesh(m, fmt);
		}
	}

	fclose(fmt);

	printf("%d meshes were loaded\n", m->nMesh);
	printf("Computing the tangent space...\n");
	ComputeTangentSpace(m);
	printf("Tangent space computed.\n");

	return 0;
}
