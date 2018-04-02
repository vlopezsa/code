#include "Platform/Log.h"
#include "Platform/Platform.h"

#include "Core/Math.h"

#include "GraphicsDX11.h"

#include <windows.h>
#include <windowsx.h>
#include <dxgi1_4.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#include <d2d1effects_2.h>
#include <DirectXColors.h>
#include <DirectXMath.h>

// include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")

GraphicsDX11::GraphicsDX11()
{
	m_Device	= nullptr;
	m_deviceCtx = nullptr;
	m_swapChain = nullptr;

	m_depthStencilView = nullptr;

	m_depthStencilTexture = nullptr;
}

void GraphicsDX11::Init()
{
	this->CreateDevice();

	LOG("Graphics Device DirectX11 Initiliazed.");
}

void GraphicsDX11::CreateDevice()
{
	UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
	flags |= D3D11_CREATE_DEVICE_DEBUGGABLE;
#endif

	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	HRESULT hr;

	hr = D3D11CreateDevice(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		flags,
		levels,
		2,
		D3D11_SDK_VERSION,
		&m_Device,
		&m_featureLevel,
		&m_deviceCtx
	);

	if (FAILED(hr))
	{
		ERR("Failed to create DX11 device");
		return;
	}

	CreateSwapChain();
}

void GraphicsDX11::CreateSwapChain()
{
	IDXGIDevice	 *dxgiDevice = nullptr;
	IDXGIAdapter *dxgiAdapter = nullptr;
	IDXGIFactory *dxgiFactory = nullptr;

	ID3D11Texture2D *backBuffer;

	D3D11_TEXTURE2D_DESC depthDesc = { 0 };
	D3D11_TEXTURE2D_DESC backDesc = { 0 };

	HRESULT hr;

	HWND hWnd = static_cast<HWND>(Platform::Instance().GetWindowHandle());

	if (hWnd == NULL)
	{
		ERR("Failed to create SwapChain. No Window has been created.");
		return;
	}

	/* Get DXGI Handlers */
	{
		hr = m_Device->QueryInterface(__uuidof(IDXGIDevice), (void**)& dxgiDevice);

		if (FAILED(hr))
		{
			ERR("Failed to querey DXGI device");
			return;
		}

		hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)& dxgiAdapter);

		if (FAILED(hr))
		{
			ERR("Failed to querey DXGI adapter");
			return;
		}

		hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)& dxgiFactory);

		if (FAILED(hr))
		{
			ERR("Failed to query DXGI factory");
			return;
		}
	}

	/* Create Swap Chain */
	{
		DXGI_SWAP_CHAIN_DESC desc = { 0 };

		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = 1;
		desc.OutputWindow = hWnd;
		desc.SampleDesc.Count = 4;
		desc.Windowed = true;

		hr = dxgiFactory->CreateSwapChain(m_Device, &desc, &m_swapChain);

		if (FAILED(hr))
		{
			ERR("Failed to create swap chain");
			return;
		}
	}

	m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&backBuffer);

	m_Device->CreateRenderTargetView(backBuffer, NULL, &m_targetView);

	backBuffer->GetDesc(&backDesc);

	depthDesc.Width = backDesc.Width;
	depthDesc.Height = backDesc.Height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	hr = m_Device->CreateTexture2D(&depthDesc, NULL, &m_depthStencilTexture);

	if (FAILED(hr))
	{
		ERR("Failed to create depth/stencil texture");
		return;
	}

	hr = m_Device->CreateDepthStencilView(m_depthStencilTexture, 0, &m_depthStencilView);

	if (FAILED(hr))
	{
		ERR("Failed to create depth/stencil view");
		return;
	}

	m_deviceCtx->OMSetRenderTargets(1, &m_targetView, m_depthStencilView);

	backBuffer->Release();

	dxgiDevice->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();

	RECT rc;

	GetWindowRect(hWnd, &rc);

	D3D11_VIEWPORT viewPort = {0};

	viewPort.Height = static_cast<float>(rc.bottom);
	viewPort.Width = static_cast<float>(rc.right);
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	m_deviceCtx->RSSetViewports(1, &viewPort);
}

void GraphicsDX11::Resize()
{
	if (!m_Device || !m_deviceCtx)
	{
		ERR("Attempted to resize graphics devices before been initialized.");
		return;
	}

	if (!m_swapChain)
		CreateSwapChain();

}

void GraphicsDX11::Release()
{
	if (m_depthStencilTexture)
		m_depthStencilTexture->Release();

	if (m_depthStencilView)
		m_depthStencilView->Release();

	if (m_swapChain)
		m_swapChain->Release();

	if (m_Device)
		m_Device->Release();

	if (m_deviceCtx)
		m_deviceCtx->Release();

	m_Device	= nullptr;
	m_deviceCtx = nullptr;
	m_swapChain = nullptr;
}

void GraphicsDX11::Present()
{
	if (!m_swapChain || !m_deviceCtx)
	{
		return;
	}

	m_swapChain->Present(0, 0);
}

void GraphicsDX11::Update()
{
	if (!m_swapChain || !m_deviceCtx)
	{
		return;
	}

	float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };

	m_deviceCtx->ClearRenderTargetView(m_targetView, clearColor);

	m_deviceCtx->ClearDepthStencilView(m_depthStencilView, 0, 0.0f, 0);
}
