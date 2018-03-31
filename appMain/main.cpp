#include "Game/Game.h"

int main()
{
	Game::Instance().Initialize();

	Game::Instance().Run();

	return 0;
}