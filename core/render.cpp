#include "render.h"
#include "image.h"

#include <windows.h>
#include <GL/glew.h>
#include <GL/glut.h>



Render::Render()
{
    preComputedEnvLight = false;
}

Render::~Render()
{
    
}

void Render::clear(Vector3 color)
{
    glClearColor(color.r, color.g, color.b, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
}

void Render::swapBuffers()
{
    glutSwapBuffers();
}

uint32_t Render::_addGPUTexture(uint32_t texIdx)
{
    GLuint glTexID=(uint32_t)-1;
    const Image *img = scene->texture.getTextureImage(texIdx);

    if (img)
    {
        glGenTextures(1, &glTexID);

        glBindTexture(GL_TEXTURE_2D, glTexID);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGB,
            img->Width,
            img->Height,
            0,
            GL_BGR,
            GL_UNSIGNED_BYTE,
            img->Data
            );
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    return glTexID;
}


void Render::updateCamera(Camera * cam)
{
    cam->SetPerspective();
}

void Render::usePreComputedEnvLight(bool enable)
{
    preComputedEnvLight = enable;

    if(preComputedEnvLight)
        glEnableClientState(GL_COLOR_ARRAY);
    else
        glDisableClientState(GL_COLOR_ARRAY);
}

float light_position[] = { 10.0f, 10.0f, 10.0f };
float light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };

float mat_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };

void Render::renderMesh(Mesh * m)
{
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    /* Setup Material */
    setupMaterial(&scene->material[m->materialIdx]);

    /* Setup Geometry */
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &m->vertex[0].position.d);
    glNormalPointer(GL_FLOAT, sizeof(Vertex), &m->vertex[0].normal.d);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &m->vertex[0].tex.d);

    // Expecting the environment light intensity to be in the diffuse color;
    if (preComputedEnvLight)
        glColorPointer(3, GL_FLOAT, sizeof(Vertex), &m->vertex[0].diffuse.d);

    glDrawElements(GL_TRIANGLES, (GLint)m->numIndices(), GL_UNSIGNED_INT, &m->index[0]);

    glClientActiveTextureARB(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Render::setupMaterial(Material * mat)
{
    mat_diffuse[0] = mat->Color.diffuse.r;
    mat_diffuse[1] = mat->Color.diffuse.g;
    mat_diffuse[2] = mat->Color.diffuse.b;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_diffuse);

    if (mat->getNumTexDiffuse() > 0)
    {
        uint32_t difIdx = mat->texIdx.diffuse[0];

        if (gpuTex.find(difIdx) == gpuTex.end())
        {
            gpuTex[difIdx] = _addGPUTexture(difIdx);
        }

        glClientActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gpuTex[difIdx]);
    }
    else
    {
        glColor3f(mat->Color.diffuse.r, mat->Color.diffuse.g, mat->Color.diffuse.b);
    }
}

void Render::renderScene(Scene * s)
{
    /* setup global scene handler */
    this->scene = s;

    if (preComputedEnvLight)
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
    }
    else
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
    }
    
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);

    /* activate gl state for array drawing */
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    /* draw all of the meshes*/
    for (uint32_t i = 0; i < s->numMeshes(); i++)
    {
        renderMesh(&s->mesh[i]);
    }
}
