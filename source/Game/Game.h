#pragma once
class Game
{
public:
	enum GAME_STATUS
	{
		GAME_BOOTING = 0,
		GAME_READY,
		GAME_RUNNING,
		GAME_EXITING,
		GAME_EXIT = 0xffff
	};

private:
	GAME_STATUS			m_Status;

public:
	void Run();

	void Initialize();

	void Release();

	GAME_STATUS getStatus() { return m_Status; }

	void setStatus(GAME_STATUS nStatus);

private:

	void Update();

private:
	Game() {}
	~Game() {}

public:
	static Game& Instance()
	{
		static Game game;

		return game;
	}

	Game(Game const&) = delete;
	void operator=(Game const&) = delete;
	 
};

