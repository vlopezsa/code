#include <assimp\scene.h>

#include "mesh.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::setNumVertices(int numVertex)
{
    try {
        this->vertex.resize(numVertex);
        this->index.resize(numVertex);
    }
    catch (std::exception &e)
    {
        throw e;
    }
}

void Mesh::setNumTriangles(int numTriangle)
{
    try
    {
        this->triangle.resize(numTriangle);
    }
    catch (std::exception &e)
    {
        throw e;
    }
}

void Mesh::addVertex(int pos, Vertex & v)
{
    try
    {
        vertex[pos].diffuse  = v.diffuse;
        vertex[pos].normal   = v.normal;
        vertex[pos].position = v.position;

        //vertex[pos].sh_coeff = v.sh_coeff;
    }
    catch (std::exception &e)
    {
        throw e;
    }
}

void Mesh::addTriangle(int pos, Triangle & t)
{
    try
    {
        triangle[pos].v1 = t.v1;
        triangle[pos].v2 = t.v2;
        triangle[pos].v3 = t.v3;
    }
    catch (std::exception &e)
    {
        throw e;
    }
}

void Mesh::importAIMesh(const aiMesh *mesh)
{
    try 
    {
        vertex.clear();
        index.clear();
        triangle.clear();

        vertex.resize(mesh->mNumVertices);
        triangle.resize(mesh->mNumFaces);
        index.resize(mesh->mNumFaces * 3);

        unsigned int texCoordIdx = AI_MAX_NUMBER_OF_TEXTURECOORDS;
        unsigned int colorIdx = AI_MAX_NUMBER_OF_COLOR_SETS;

        for (unsigned int i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; i++)
        {
            if (mesh->HasTextureCoords(i))
            {
                texCoordIdx = i;
                break;
            }
        }

        for (unsigned int i = 0; i < AI_MAX_NUMBER_OF_COLOR_SETS; i++)
        {
            if (mesh->HasVertexColors(i))
            {
                colorIdx = i;
                break;
            }
        }

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            vertex[i].position.x = mesh->mVertices[i].x;
            vertex[i].position.y = mesh->mVertices[i].y;
            vertex[i].position.z = mesh->mVertices[i].z;

            if (mesh->HasNormals())
            {
                vertex[i].normal.x = mesh->mNormals[i].x;
                vertex[i].normal.y = mesh->mNormals[i].y;
                vertex[i].normal.z = mesh->mNormals[i].z;
            }

            if (texCoordIdx < AI_MAX_NUMBER_OF_TEXTURECOORDS)
            {
                vertex[i].tex.u = mesh->mTextureCoords[texCoordIdx][i].x;
                vertex[i].tex.v = mesh->mTextureCoords[texCoordIdx][i].y;
            }

            if (colorIdx < AI_MAX_NUMBER_OF_COLOR_SETS)
            {
                vertex[i].diffuse.r = mesh->mColors[colorIdx][i].r;
                vertex[i].diffuse.g = mesh->mColors[colorIdx][i].g;
                vertex[i].diffuse.b = mesh->mColors[colorIdx][i].b;
            }
        }

        if (mesh->HasFaces())
        {
            for (unsigned int i = 0, j=0; i < mesh->mNumFaces; i++)
            {
                triangle[i].v1 = mesh->mFaces[i].mIndices[0];
                triangle[i].v2 = mesh->mFaces[i].mIndices[1];
                triangle[i].v3 = mesh->mFaces[i].mIndices[2];

                index[j++] = mesh->mFaces[i].mIndices[0];
                index[j++] = mesh->mFaces[i].mIndices[1];
                index[j++] = mesh->mFaces[i].mIndices[2];
            }
        }        

        materialIdx = mesh->mMaterialIndex;
    }
    catch (std::exception &e)
    {
        throw e;
    }   
}
