#pragma once

#include <windows.h>
#include <windowsx.h>
#include <dxgi1_4.h>
#include <d3d11_3.h>

#include "Graphics/Graphics.h"

class GraphicsDX11 : public Graphics
{
public:
	void Init();

	void Release();

	void Present();

	void Update();

	void Resize();

private:
	GraphicsDX11();
	~GraphicsDX11() {}

	ID3D11Device			*m_Device;
	ID3D11DeviceContext		*m_deviceCtx;
	IDXGISwapChain			*m_swapChain;

	D3D_FEATURE_LEVEL		m_featureLevel;

	ID3D11RenderTargetView *m_targetView;

	ID3D11Texture2D		   *m_depthStencilTexture;

	ID3D11DepthStencilView *m_depthStencilView;

	void CreateDevice();

	void CreateSwapChain();

public:
	static GraphicsDX11& Instance()
	{
		static GraphicsDX11 gfx;

		return gfx;
	}

	GraphicsDX11(GraphicsDX11 const&) = delete;
	void operator=(GraphicsDX11 const&) = delete;
};
