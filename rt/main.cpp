#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <GL/glut.h>
#include <AntTweakBar.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "render.h"
#include "camera.h"
#include "graphics.h"
#include "osutil.h"
#include "scene.h"
#include "thirdparty.h"
#include "montecarlo.h"

using namespace std;

/* Window related variables */
char *g_strAppTitle = "Ray Tracer";
int g_winWidth = 800;
int g_winHeight = 800;
int g_winGlutID;

/* Render related variables */
Render g_Render;

/* Scene related variables */
Scene g_Scene, g_SceneRT;

/* Camera related variables */
Camera *g_Camera = &g_Scene.camera;
float camSpeed = 0.5f;
bool MouseDown = false;
int LastMouseX, LastMouseY, CurMouseX, CurMouseY;
int LastMouseButtonClicked;

/* debug options */
bool  g_drawRays = false;
float g_rayPercentage = 10.0;

#include "triangleobj.h"

enum OPT_OPTIONS 
{
	OPT_DIFFUSE_ONLY,
	OPT_SHADOWS_ONLY,
	OPT_FULL_PATHTRACER
};

typedef struct {
	uint32_t m_Width;
	uint32_t m_Height;

	struct Camera {
		Vector3 m_Position;

		Vector3 m_LookAt;

		Vector3 m_Up;

		float m_Fovy;

		std::vector<Vector2> m_Offsets;
	} m_Camera;

	struct Light
	{
		Vector3 m_Position;
		float   m_Radius;
		float   m_Intensity;
	} m_Light;

	uint32_t m_Operation;

	std::string m_InputFile;
	std::string m_OutputFile;

	uint8_t m_Interactive;
	uint8_t m_Verbosity;
}Config;

typedef struct {
    Vector3 o;
    Vector3 e;
    bool hit;
}Line;

Config g_Config = { 0 };

Sampler *g_Sampler = new MonteCarlo(32 * 32);

std::vector<Line> lines;

float toRadians(float deg)
{
	return (M_PI * deg) / 180.f;
}

Vector3 toCartesian(Vector2 &spherical)
{
	Vector3 cartesian;

	cartesian.x = (float)(sin(spherical.theta) * cos(spherical.phi));
	cartesian.y = (float)(sin(spherical.theta) * sin(spherical.phi));
	cartesian.z = (float)(cos(spherical.theta));

	return cartesian;
}

inline Ray RTCCreateCameraRay(
	Vector3 &Position,
	Vector3 &camera_u, Vector3 &camera_v,
	Vector3 &camera_dir,
	uint32_t x, uint32_t y,
	uint32_t Width, uint32_t Height,
	float fov, float offX, float offY)
{
	float u = ((float)x + offX) / (float)(Width - 1) - 0.5f;
	float v = ((float)(Height - 1 - y) + offY) / (float)(Height - 1) - 0.5f;
	
	// This is only valid for square aspect ratio images
	Vector3 rayDir = camera_u * u + camera_v * v + camera_dir * fov;
	rayDir.normalize();
	Ray ray(Position, rayDir);

	return ray;
}

float RTCGetOcclusion1(Ray &ray, Sampler *sampler)
{
	float occlusion = 0.0f;

	IntersectionInfo I = {0};

	bool hit = g_Scene.bvh->getIntersection(ray, &I, false);

	if (!hit)
		return 0.0f;

	TriangleObj *tr = (TriangleObj *)I.object;

	Vector4 n = tr->getNormal(I);

	return I.t;

	/*for (uint32_t si = 0; si < sampler->numSamples; si++)
	{
		float costerm = n * sampler->Samples[si].Cartesian;

		// only the hemisphere pointer toward the normal
		if (costerm > 0.0f)
		{
			Vector4 bPos = I.hit + (sampler->Samples[si].Cartesian*1.1);
			Ray bRay(bPos, sampler->Samples[si].Cartesian);

			IntersectionInfo It;

			bool hitt = g_Scene.bvh->getIntersection(bRay, &It, true);

			if (hitt)
				occlusion += 1.0f;
		}
	}

	return occlusion / (float)(sampler->numSamples);*/
}

Vector4 RTCGetLightDirAsPoint(IntersectionInfo &intInfo)
{
	return normalize(Vector4(g_Config.m_Light.m_Position) - intInfo.hit);
}

// -1.0 - 1.0
float rndFloat()
{
	return ((float)rand() / ((float)RAND_MAX / 2.0f)) - 1.0f;
}

Vector4 RTCGetLightDirAsArea(IntersectionInfo &intInfo)
{
	Vector4 rndPos;
	float radius = g_Config.m_Light.m_Radius;

	rndPos.x = g_Config.m_Light.m_Position.x + radius * rndFloat();
	rndPos.y = g_Config.m_Light.m_Position.y + radius * rndFloat();
	rndPos.z = g_Config.m_Light.m_Position.z + radius * rndFloat();

	return normalize(rndPos - intInfo.hit);
}

float RTCOcclusionByLight(IntersectionInfo &intInfo, uint32_t nRays)
{
	uint32_t count = 0;

	for (uint32_t i = 0; i < nRays; i++)
	{
		Ray lightRay(intInfo.hit, RTCGetLightDirAsArea(intInfo));

		IntersectionInfo I;

		if (g_Scene.bvh->getIntersection(lightRay, &I, true))
			count++;
	}
	return (float)count / (float)nRays;
}

typedef struct WorkTile_t
{
	Image*		img_out;
	uint32_t	start_x, start_y;
	uint32_t	end_x, end_y;
	float		fov;
	Vector3		camera_pos;
	Vector3		camera_dir;
	Vector3		camera_u;
	Vector3		camera_v;

	uint32_t	num_rays;

	uint32_t	progress;
	
	bool		notified;
	bool		ready;
	bool		done;

	std::mutex				mtx;
	std::condition_variable	cond;

	WorkTile_t()
	{

	}

	WorkTile_t(const WorkTile_t &w)
	{
		img_out = w.img_out;
		start_x = w.start_x;
		start_y = w.start_y;
		end_x = w.end_x;
		end_y = w.end_y;
		fov = w.fov;
		camera_pos = w.camera_pos;
		camera_dir = w.camera_dir;
		camera_u = w.camera_u;
		camera_v = w.camera_v;

		num_rays = w.num_rays;

		progress = w.progress;

		ready = w.ready;
		notified = w.notified;
		done = w.done;
	}

};

bool g_Ready = false;

void RTCTileExecute(WorkTile_t *work)
{
	IntersectionInfo intInfo;

	work->progress = 0;

	uint8_t *out = work->img_out->getData();

	uint32_t Width = g_Config.m_Width;
	uint32_t Height = g_Config.m_Height;

	while (!work->done)
	{
		std::unique_lock<std::mutex> lck(work->mtx);

		while (!work->notified) work->cond.wait(lck);

		if (work->done)
			break;

		work->notified = false;

		for (size_t j = work->start_y; j < work->end_y && j < Height; j++)
		{
			size_t index = j * work->img_out->Pitch + (work->start_x * sizeof(float) * 3);

			float *pixel = (float *)&out[index];

			for (size_t i = work->start_x, k = 0; i < work->end_x && i < Width; i++, k += 3)
			{
				float depth = 0.0f;
				float occlusion = 0.0f;
				float mask = 0.0f;

				for (auto &offset : g_Config.m_Camera.m_Offsets)
				{
					Ray camRay = RTCCreateCameraRay(
						work->camera_pos, work->camera_u, work->camera_v, work->camera_dir,
						i, j, Width, Height, work->fov, offset.x, offset.y);

					intInfo.reset();

					if (g_Scene.bvh->getIntersection(camRay, &intInfo, false))
					{
						depth += intInfo.t;

						Vector4 newOrig = intInfo.hit;
						Vector4 hitNormal = intInfo.object->getNormal(intInfo);
						Vector4 lightDir = RTCGetLightDirAsPoint(intInfo);

						Ray lightRay(newOrig + (hitNormal * 0.01f), lightDir);

						IntersectionInfo I;

						if (g_Scene.bvh->getIntersection(lightRay, &I, true))
						{
							mask = 1.0f;
						}

						occlusion += RTCOcclusionByLight(intInfo, work->num_rays);
					}
				}

				pixel[k + 0] = depth / (float)g_Config.m_Camera.m_Offsets.size();
				pixel[k + 1] = occlusion / (float)g_Config.m_Camera.m_Offsets.size();
				pixel[k + 2] = mask;

				work->progress++;
			}
		}

		work->ready = true;

		g_Ready = true;
	}
}

void RTCExecute()
{
	if (g_Scene.bvh == NULL)
		return;

	Image *imgOut = new Image(g_Config.m_Width, g_Config.m_Height, ImagePixelFormat::Img_Pixel_RGBF, 0);

	uint32_t maxLightRays = 1000;

	/* camera setup */
	float fov = 0.5f / tanf(g_Config.m_Camera.m_Fovy * 3.14159265*.5f / 180.f);

	Vector3 Position(g_Config.m_Camera.m_Position.x, g_Config.m_Camera.m_Position.y, g_Config.m_Camera.m_Position.z);
	Vector3 LookAt(g_Config.m_Camera.m_LookAt.x, g_Config.m_Camera.m_LookAt.y, g_Config.m_Camera.m_LookAt.z);
	Vector3 Up(g_Config.m_Camera.m_Up.x, g_Config.m_Camera.m_Up.y, g_Config.m_Camera.m_Up.z);

	Vector3 camera_dir = LookAt - Position;
	camera_dir.normalize();

	Vector3 camera_u = cross(Up, camera_dir);
	camera_u.normalize();

	Vector3 camera_v = cross(camera_dir, camera_u);
	camera_v.normalize();

	camera_u = camera_u * -1.0f;
	camera_v = camera_v * -1.0f;

	/* thread setup */
	uint32_t numCpus = std::thread::hardware_concurrency();
	uint32_t workSplit;
	uint32_t numPixels = g_Config.m_Width * g_Config.m_Height;

	if (numCpus <= 1)
		workSplit = 1;
	else
		workSplit = numCpus;

	std::vector<WorkTile_t> work;

	work.resize(workSplit);

	for (uint32_t i = 0; i < workSplit; i++)
	{
		work[i].img_out = imgOut;
		work[i].progress = 0;
		work[i].camera_dir = camera_dir;
		work[i].camera_u = camera_u;
		work[i].camera_v = camera_v;
		work[i].camera_pos = Position;
		work[i].fov = fov;
		work[i].num_rays = maxLightRays;
		work[i].done = false;
		work[i].ready = true;
		work[i].notified = false;
	}

	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.1f, 0.1f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.3f, 0.1f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.5f, 0.1f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.7f, 0.1f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.9f, 0.1f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.2f, 0.3f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.4f, 0.3f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.6f, 0.3f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.8f, 0.3f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.1f, 0.5f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.3f, 0.5f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.5f, 0.5f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.7f, 0.5f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.9f, 0.5f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.2f, 0.7f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.4f, 0.7f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.6f, 0.7f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.8f, 0.7f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.1f, 0.9f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.3f, 0.9f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.5f, 0.9f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.7f, 0.9f));
	g_Config.m_Camera.m_Offsets.push_back(Vector2(0.9f, 0.9f));

	std::vector<std::thread> workThreads;

	for (uint32_t i = 0; i < workSplit; i++)
	{
		workThreads.push_back(std::thread(RTCTileExecute, &work[i]));
	}

	g_Ready = true;

	uint32_t tile_size = 64;

	for (uint32_t y=0; y<g_Config.m_Height; y+= tile_size)
	{
		for (uint32_t x = 0; x<g_Config.m_Width;)
		{
			while (!g_Ready) std::this_thread::yield();

			g_Ready = false;

			for (uint32_t i = 0; i < workSplit && x < g_Config.m_Width; i++)
			{
				if (work[i].ready == true)
				{
					work[i].mtx.lock();

					work[i].start_x = x;
					work[i].start_y = y;
					work[i].end_x = std::min(x + tile_size, g_Config.m_Width);
					work[i].end_y = std::min(y + tile_size, g_Config.m_Height);

					work[i].notified = true;
					work[i].ready = false;

					work[i].cond.notify_one();

					work[i].mtx.unlock();

					x += tile_size;
				}
			}

			uint32_t progress = 0;

			for (uint32_t i = 0; i < workSplit; i++)
			{
				progress += work[i].progress;
			}

			std::cout << "Progress " << ((float)progress / (float)numPixels) * 100.0f << "%\r";
		}
	}

	for (uint32_t i = 0; i < workSplit; i++)
	{
		work[i].mtx.lock();

		work[i].done = true;
		work[i].notified = true;

		work[i].cond.notify_one();

		work[i].mtx.unlock();
	}

	for (uint32_t i = 0; i < workSplit; i++)
	{
		workThreads[i].join();
	}

	std::cout << "Progress 100.00%\n";

	imgOut->SaveToFile(g_Config.m_OutputFile.c_str());

	delete imgOut;
}

void RTCExecute1()
{
	if (g_Scene.bvh == NULL)
		return;

	Image *imgOut= new Image(g_Config.m_Width, g_Config.m_Height, ImagePixelFormat::Img_Pixel_RGBF, 0);

	IntersectionInfo intInfo;

	uint8_t *out = imgOut->getData();

	uint32_t Width = g_Config.m_Width;
	uint32_t Height = g_Config.m_Height;

	uint32_t maxLightRays = 100;

	float fov = 0.5f / tanf(g_Config.m_Camera.m_Fovy * 3.14159265*.5f / 180.f);

	Vector3 Position(g_Config.m_Camera.m_Position.x, g_Config.m_Camera.m_Position.y, g_Config.m_Camera.m_Position.z);
	Vector3 LookAt(g_Config.m_Camera.m_LookAt.x, g_Config.m_Camera.m_LookAt.y, g_Config.m_Camera.m_LookAt.z);
	Vector3 Up(g_Config.m_Camera.m_Up.x, g_Config.m_Camera.m_Up.y, g_Config.m_Camera.m_Up.z);

	Vector3 camera_dir = LookAt - Position;
	camera_dir.normalize();

	Vector3 camera_u = cross(Up, camera_dir);
	camera_u.normalize();

	Vector3 camera_v = cross(camera_dir, camera_u);
	camera_v.normalize();

	camera_u = camera_u * -1.0f;
	camera_v = camera_v * -1.0f;

	std::vector<Vector2> offList;
	offList.push_back(Vector2(0.5, 0.5));

	for (size_t j = 0, ridx = 0; j< Height; j++) {
		size_t index = j * imgOut->Pitch;

		float *pixel = (float *)&out[index];

		for (size_t i = 0, k = 0; i< Width; i++, k += 3)
		{

			Ray camRay = RTCCreateCameraRay(
				Position, camera_u, camera_v, camera_dir,
				i, j, Width, Height,
				fov, offList[0].x, offList[0].y);

			intInfo.reset();

			float depth = 0.0f;
			float occlusion = 0.0f;
			float mask = 0.0f;

			if (g_Scene.bvh->getIntersection(camRay, &intInfo, false))
			{
				depth = intInfo.t;

				Vector4 newOrig = intInfo.hit;
				Vector4 hitNormal = intInfo.object->getNormal(intInfo);
				Vector4 lightDir = RTCGetLightDirAsPoint(intInfo);

				Ray lightRay(newOrig + (hitNormal * 0.01f), lightDir);

				//intInfo.reset();

				IntersectionInfo I;

				if (g_Scene.bvh->getIntersection(lightRay, &I, true))
				{
					mask = 1.0f;
				}

				occlusion = RTCOcclusionByLight(intInfo, maxLightRays);
			}

			pixel[k + 0] = depth;
			pixel[k + 1] = occlusion;
			pixel[k + 2] = mask;
		}
	}

	imgOut->SaveToFile(g_Config.m_OutputFile.c_str());

	delete imgOut;
}

void UpdateEyePositionFromMouse()
{
    Vector3 pt = { 0,0,0 };
    GLfloat dx, dy;

    if (MouseDown == false)
        return;

    switch (LastMouseButtonClicked)
    {
    case GLUT_LEFT_BUTTON:
        dx = (float)(CurMouseX - LastMouseX);
        dy = (float)(CurMouseY - LastMouseY);

        g_Camera->ChangeHeading(0.2f * dx);
        g_Camera->ChangePitch(0.2f * dy);
        break;

    case GLUT_RIGHT_BUTTON:
        break;
    }

    // Update the last position of the mouse
    LastMouseX = CurMouseX;
    LastMouseY = CurMouseY;
}

/* GLUT callback functions */

void drawRays()
{
    glDisable(GL_LIGHTING);

    float fi = 0.0f;
    float skip = (uint32_t)(float)1.0f / (g_rayPercentage / 100.0f);

    if (skip<=0.00001) skip = 1.0f;

    glBegin(GL_LINES);

    for (uint32_t i=0, fi=0.0f; i < lines.size();)
    {
        if(lines[i].hit)
            glColor3f(0.0f, 1.0f, 0.0f);
        else
            glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(
            lines[i].o.x, lines[i].o.y, lines[i].o.z
            );
        glVertex3f(
            lines[i].e.x, lines[i].e.y, lines[i].e.z
            );

        fi += skip;
        i = (uint32_t)fi;
    }

    glEnd();
}

void RenderF()
{
    g_Render.clear({ 0.1f, 0.1f, 0.0f });

    UpdateEyePositionFromMouse();

    g_Render.updateCamera(g_Camera);

    g_Render.renderScene(&g_Scene);

    if(g_drawRays)
        drawRays();

    TwDraw();

    g_Render.swapBuffers();
}

void ChangeSize(GLsizei w, GLsizei h) {
    // Prevent division by 0
    if (h == 0) h = 1;

    // Store window's width and height.
    g_winWidth = w;
    g_winHeight = h;
    // Set the viewport.
    glViewport(0, 0, w, h);

    // Reset transform
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(40.0, (GLdouble)w / (GLdouble)h, 0.6, 20000.0);
    glMatrixMode(GL_MODELVIEW);

    TwWindowSize(w, h);
}

void KeyEvent(uint8_t key, int x, int y)
{
    if (TwEventKeyboardGLUT(key, x, y))
        return;

    switch (key)
    {
    break;
    case 'w': case 'W':
        g_Camera->ChangeVelocity(camSpeed);
        break;
    case 's': case 'S':
        g_Camera->ChangeVelocity(camSpeed*-1.0f);
        break;
    case 'a': case 'A':
    {
        float Heading = (float)((g_Camera->m_HeadingDegrees - 90.0f) / 180.0f * M_PI);
        float x = sin(Heading);
        float z = cos(Heading);

        g_Camera->m_Position.x += x*camSpeed;
        g_Camera->m_Position.z += z*camSpeed;
    }
    break;

    case 'd': case 'D':
    {
        float Heading = (float)((g_Camera->m_HeadingDegrees + 90.0f) / 180.0f * M_PI);
        float x = sin(Heading);
        float z = cos(Heading);

        g_Camera->m_Position.x += x*camSpeed;
        g_Camera->m_Position.z += z*camSpeed;
    }
    break;
    }
}

void MouseFunc(int button, int state, int x, int y)
{
    if (TwEventMouseButtonGLUT(button, state, x, y))
        return;

    if (GLUT_DOWN == state)
    {
        CurMouseX = LastMouseX = x;
        CurMouseY = LastMouseY = y;
        LastMouseButtonClicked = button;
        MouseDown = true;
    }
    else
    {
        MouseDown = false;
    }
}

void MotionFunc(int x, int y)
{
    if (TwEventMouseMotionGLUT(x, y))
        return;

    CurMouseX = x;
    CurMouseY = y;
}

void IdleFunc()
{
    glutPostRedisplay();
}

/* Application function */

void glutSetup()
{
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(g_winWidth, g_winHeight);

    g_winGlutID = glutCreateWindow(g_strAppTitle);

    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderF);
    glutKeyboardFunc(KeyEvent);
    glutMouseFunc(MouseFunc);
    glutMotionFunc(MotionFunc);
    glutPassiveMotionFunc(MotionFunc);
    glutIdleFunc(IdleFunc);
}

void cameraSetup()
{
    g_Camera->m_MaxForwardVelocity = 100.0f;
    g_Camera->m_MaxPitchRate = 5.0f;
    g_Camera->m_MaxHeadingRate = 5.0f;
    g_Camera->m_PitchDegrees = 18.4;
    g_Camera->m_HeadingDegrees = 45.2;

    g_Camera->m_Position.x = -15.7551;
    g_Camera->m_Position.y = 6.6482;
    g_Camera->m_Position.z = -15.7551;
}

void toolBoxSetup()
{
    TwBar *bar = TwNewBar("Info & Options");
    TwDefine(" GLOBAL help='PRT Simple Example' ");
    TwDefine(" 'Info & Options' size='200 200' color='96 216 224' ");

    TwAddSeparator(bar, "cam separator", "group='Camera'");

    TwAddVarRW(bar, "CamSpeed", TW_TYPE_FLOAT, &camSpeed, "group='Camera'");
    TwAddVarRW(bar, "CamPosX", TW_TYPE_FLOAT, &g_Camera->m_Position.x, "group='Camera'");
    TwAddVarRW(bar, "CamPosY", TW_TYPE_FLOAT, &g_Camera->m_Position.y, "group='Camera'");
    TwAddVarRW(bar, "CamPosZ", TW_TYPE_FLOAT, &g_Camera->m_Position.z, "group='Camera'");

    TwAddSeparator(bar, "ray separator", "group='Rays'");
    TwAddVarRW(bar, "Draw Rays", TW_TYPE_BOOL8, &g_drawRays, "group='Rays'");
    TwAddVarRW(bar, "Rays Percentage", TW_TYPE_FLOAT, &g_rayPercentage, "group='Rays'");
}

void CleanUp()
{
    endThirdParty();
}

void fillUpBuffer(Image *img)
{
    unsigned int x, y;
    uint8_t *buf = img->getData();

    for (y = 0; y < img->Height; y++)
    {
        for (x = 0; x < img->Width; x++)
        {
            buf[0] = (uint8_t)(((float)x / (float)img->Width) * 255.0f);
            buf[1] = (uint8_t)(((float)y / (float)img->Height) * 255.0f);
            buf[2] = (uint8_t)(((float)(img->Width-x) / (float)img->Width) * 255.0f);

            buf += 3;
        }
    }
}

void initRTScene()
{
    /* Creating our frame buffer */
    Image *img = new Image(800, 800, ImagePixelFormat::Img_Pixel_RGB, 24);
    unsigned int texIdx=g_SceneRT.texture.addTextureFromImg(img, "texFrameBuffer");

    fillUpBuffer(img);

    /* Creating material out of frame buffer */
    g_SceneRT.setNumMaterials(1);
    Material *mat = &g_SceneRT.material[0];

    mat->texIdx.diffuse.push_back(texIdx);

    /* Creating quad to display our framebuffer */
    g_SceneRT.setNumMeshes(1);
    Mesh *m = &g_SceneRT.mesh[0];

    m->materialIdx = 0;

    m->setNumVertices(4);
    m->setNumTriangles(2);

    // Vertices
    m->vertex[0].position = Vector3(-1.0f, 1.0f, 0.0f);
    m->vertex[1].position = Vector3( 1.0f, 1.0f, 0.0f);
    m->vertex[2].position = Vector3( 1.0f,-1.0f, 0.0f);
    m->vertex[3].position = Vector3(-1.0f,-1.0f, 0.0f);

    m->vertex[0].tex = Vector2(0.0f, 0.0f);
    m->vertex[1].tex = Vector2(1.0f, 0.0f);
    m->vertex[2].tex = Vector2(1.0f, 1.0f);
    m->vertex[3].tex = Vector2(0.0f, 1.0f);

    m->vertex[0].diffuse = Vector3(1.0f, 1.0f, 1.0f);
    m->vertex[1].diffuse = Vector3(1.0f, 1.0f, 1.0f);
    m->vertex[2].diffuse = Vector3(1.0f, 1.0f, 1.0f);
    m->vertex[3].diffuse = Vector3(1.0f, 1.0f, 1.0f);

    // Indices
    m->index[0] = 0;
    m->index[1] = 1;
    m->index[2] = 2;
    m->index[3] = 0;
    m->index[4] = 2;
    m->index[5] = 3;
}



void processOffline()
{
	try {
		g_Scene.loadFromFile((char *)&g_Config.m_InputFile.c_str()[0]);
		g_Scene.buildBVH();

		if(g_Config.m_Verbosity)
			osDisplaySceneInfo(&g_Scene);

		RTCExecute();
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}
}

// --width 800 --height 800 --cam_pos "10.12 10.12 10.12" --cam_fovy 30.00 --cam_up "0.0 1.0 0.0" --cam_lookat "0.0 0.0 0.0" --light_pos "-0.842 -0.428 0.329" --output output.exr --input "D:\Serious\Models\SimpleScene\simple_low_div.obj"
// --width 800 --height 800 --cam_pos "15.396 4.242 2.674" --cam_fovy 30.00 --cam_up "0.0 1.0 0.0" --cam_lookat "14.626 3.767 2.248" --light_pos "0.362 -0.479 -0.8" --output shadowset.exr --input "D:\Serious\Models\shadowSet\shadowset.obj"

void parseCmdArgs(int argc, char **argv)
{
	g_Config.m_Interactive = 1;

	if (argc <= 1)
		return;

	for (int i = 0; i < argc; i++)
	{
		if (!strcmp(argv[i], "--width"))
		{
			g_Config.m_Width = (++i<argc)? atoi(argv[i]): 0;
		}
		else if (!strcmp(argv[i], "--height"))
		{
			g_Config.m_Height = (++i<argc) ? atoi(argv[i]) : 0;
		}
		else if (!strcmp(argv[i], "--input"))
		{
			g_Config.m_InputFile = (++i<argc) ? string(argv[i]) : string("");
		}
		else if (!strcmp(argv[i], "--output"))
		{
			g_Config.m_OutputFile= (++i<argc) ? string(argv[i]) : string("");
		}
		else if (!strcmp(argv[i], "--cam_pos"))
		{
			if (++i >= argc)
				continue;
			sscanf(argv[i], "%f %f %f", &g_Config.m_Camera.m_Position.x, &g_Config.m_Camera.m_Position.y, &g_Config.m_Camera.m_Position.z);
		}
		else if (!strcmp(argv[i], "--cam_lookat"))
		{
			if (++i >= argc)
				continue;
			sscanf(argv[i], "%f %f %f", &g_Config.m_Camera.m_LookAt.x, &g_Config.m_Camera.m_LookAt.y, &g_Config.m_Camera.m_LookAt.z);
		}
		else if (!strcmp(argv[i], "--cam_up"))
		{
			if (++i >= argc)
				continue;
			sscanf(argv[i], "%f %f %f", &g_Config.m_Camera.m_Up.x, &g_Config.m_Camera.m_Up.y, &g_Config.m_Camera.m_Up.z);
		}
		else if (!strcmp(argv[i], "--cam_fovy"))
		{
			// focalLengthToFovy = 2.0f * atan(0.5f * frameHeight / focalLength);
			// default frameHeight => 24.0

			g_Config.m_Camera.m_Fovy = (++i<argc) ? atof(argv[i]) : 0;
		}
		else if (!strcmp(argv[i], "--light_pos"))
		{
			if (++i >= argc)
				continue;
			sscanf(argv[i], "%f %f %f", &g_Config.m_Light.m_Position.x, &g_Config.m_Light.m_Position.y, &g_Config.m_Light.m_Position.z);
		}
		//--light_radius 0.5 --light_intensity 1.0
		else if (!strcmp(argv[i], "--light_radius"))
		{
			g_Config.m_Light.m_Radius = (++i<argc) ? atof(argv[i]) : 0.0f;
		}
		else if (!strcmp(argv[i], "--light_intensity"))
		{
			g_Config.m_Light.m_Intensity = (++i<argc) ? atof(argv[i]) : 0.0f;
		}
		else if (!strcmp(argv[i], "--verbosity"))
		{
			g_Config.m_Verbosity = 1;
		}
	}

	g_Config.m_Interactive = 0;
}

int main(int argc, char **argv)
{
    initThirdParty(&argc, &argv);

	atexit(&CleanUp);

	parseCmdArgs(argc, argv);

	if (g_Config.m_Interactive)
	{
		cameraSetup();

		toolBoxSetup();

		initRTScene();

		glutSetup();

		// Run GLUT loop and hence the application
		glutMainLoop();
	}
	else 
	{
		processOffline();
	}

    return 0;
}