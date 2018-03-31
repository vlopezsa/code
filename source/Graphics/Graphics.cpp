#include "Graphics/Graphics.h"
#include "Graphics/DX11/GraphicsDX11.h"

void Graphics::Init()
{
}

void Graphics::Release()
{
}

void Graphics::Present()
{
}

void Graphics::Update()
{
}

void Graphics::Resize()
{
}

Graphics & Graphics::Instance()
{
	return GraphicsDX11::Instance();
}
