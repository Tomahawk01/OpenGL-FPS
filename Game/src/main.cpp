#include "Game/config.h"
#include "Utils/Formatter.h"
#include "Utils/Log.h"
#include "Utils/SystemInfo.h"

#include <Windows.h>

int main()
{
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	Game::Log::Info("Game version: {}.{}.{}", Game::Version::MAJOR, Game::Version::MINOR, Game::Version::PATCH);
	Game::Log::Info("{}", Game::GetSystemInfo());

	return 0;
}
