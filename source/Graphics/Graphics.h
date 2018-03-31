#pragma once

#include "Platform/Types.h"

class Graphics
{
	friend class GraphicsDX11;

public:
	virtual void Init();

	virtual void Release();

	virtual void Present();

	virtual void Update();

	virtual void Resize();

private:
	Graphics() {}
	virtual ~Graphics() {}

public:
	static Graphics& Instance();

	Graphics(Graphics const&) = delete;
	void operator=(Graphics const&) = delete;
};