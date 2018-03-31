#include "Game/Game.h"
#include "Render/Render.h"
#include "Graphics/Graphics.h"

#include "Platform/Platform.h"
#include "Platform/Log.h"

void Game::Initialize()
{
	setStatus(GAME_BOOTING);

	Platform::Instance().Init();

	Render::Instance().Init();

	Graphics::Instance().Init();

	setStatus(GAME_READY);

	LOG("Game Initialized.")
}

void Game::Release()
{
	LOG("Game Releasing.");

	Graphics::Instance().Release();

	Render::Instance().Release();

	Platform::Instance().Init();

	setStatus(GAME_EXIT);

	LOG("Game Released.")
}

void Game::setStatus(GAME_STATUS nStatus)
{
	switch (nStatus)
	{
		case GAME_BOOTING:
		case GAME_READY:
		case GAME_RUNNING:
		case GAME_EXITING:
		case GAME_EXIT:
			m_Status = nStatus;
			break;

		default:
			WARNING("Tried to change to an invalid GAME_STATUS (%d) - Ignored.", (int32)nStatus);
			break;
	}
}

void Game::Exit()
{
	LOG("Game Exiting Dispatched.")
	Game::Instance().setStatus(Game::GAME_EXITING);
}

void Game::Update()
{
	Platform::Instance().Update();

	Render::Instance().Update();

	Graphics::Instance().Update();

	Graphics::Instance().Present();
}

void Game::Run()
{
	setStatus(GAME_RUNNING);

	while (m_Status != GAME_EXITING && m_Status != GAME_EXIT)
	{
		Update();
	}

	Release();
}