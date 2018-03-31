#include "Platform/Platform.h"
#include "Platform/Types.h"
#include "Platform/Log.h"

#include "Core/Settings.h"

#include "Game/Game.h"

#include <iostream>

#include <windows.h>

Platform::Platform()
{
	m_Handle = nullptr;
}

LRESULT CALLBACK gWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


Platform::~Platform()
{	
}

void Platform::InitWindow(uint32 width, uint32 height, const char *title)
{
	HWND hWnd;

	WNDCLASS wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = gWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = "GameWndClass";

	RegisterClass(&wc);

	hWnd = CreateWindow("GameWndClass",
				title,
				WS_OVERLAPPEDWINDOW, 
				CW_USEDEFAULT, 
				CW_USEDEFAULT, 
				width, 
				height, 
				0, 
				0, 
				GetModuleHandle(NULL), 
				0);

	ShowWindow(hWnd, SW_SHOW);
	
	UpdateWindow(hWnd);

	m_Handle = (void *)hWnd;

}

void Platform::Init()
{
	InitWindow(Settings::Instance().getFrameWidth(),
				Settings::Instance().getFrameHeight(),
				Settings::Instance().getTitleName());

	LOG("Platform Services Initialized.")
}

void Platform::Release()
{

}

void Platform::Update()
{
	MSG msg;

	if (m_Handle == nullptr)
		return;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
		{
			Game::Instance().Exit();
		}
	}
}

void Platform::StdOut(const char *str)
{
	std::cout << str << std::endl;

	if (IsDebuggerPresent())
	{
		OutputDebugStringA(str);
		OutputDebugStringA("\n");
	}
}

void Platform::StdErr(const char *str)
{
	std::cerr << str << std::endl;

	if (IsDebuggerPresent())
	{
		OutputDebugStringA(str);
		OutputDebugStringA("\n");
	}
}