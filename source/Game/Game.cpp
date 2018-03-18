#include "Game.h"

#include "Platform/Log.h"

void Game::Initialize()
{
	setStatus(GAME_BOOTING);

	setStatus(GAME_READY);

	LOG("Game Initialized.")
}

void Game::Release()
{
	setStatus(GAME_EXIT);
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

void Game::Update()
{
	
}

void Game::Run()
{
	setStatus(GAME_RUNNING);

	while (m_Status != GAME_EXITING)
	{
		Update();
	}

	Game::Instance().Release();
}