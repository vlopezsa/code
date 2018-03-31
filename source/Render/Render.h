#pragma once

#include "Platform/Types.h"

class Render
{
public:
	void Init();

	void Release();

	void Update();

private:
	Render() {}
	~Render() {}

public:
	static Render& Instance()
	{
		static Render render;

		return render;
	}

	Render(Render const&) = delete;
	void operator=(Render const&) = delete;
};

