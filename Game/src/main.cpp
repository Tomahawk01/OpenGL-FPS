#include "Game/config.h"
#include "Graphics/Window.h"
#include "Graphics/Shader.h"
#include "Utils/Formatter.h"
#include "Utils/Log.h"
#include "Utils/SystemInfo.h"

using namespace std::literals;

namespace {

	constexpr auto sampleShader = R"(
	#version 460 core
	
	void main()
	{
		gl_Position = vec4(1.0f);
	}
	)"sv;

}

int main()
{
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	Game::Log::Info("Game version: {}.{}.{}", Game::Version::MAJOR, Game::Version::MINOR, Game::Version::PATCH);
	Game::Log::Info("{}", Game::GetSystemInfo());

	auto window = Game::Window{ Game::WindowMode::WINDOWED, 1920u, 1080u, 0u, 0u };
	auto running = true;

	auto basicVert = Game::Shader{ sampleShader, Game::ShaderType::VERTEX, "sample_shader"sv };

	while (running)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto event = window.PollEvent();
		while (event && running)
		{
			std::visit(
				[&](auto&& arg) {
					using T = std::decay_t<decltype(arg)>;

					if constexpr (std::same_as<T, Game::KeyEvent>)
					{
						Game::Log::Info("Stopping...");
						running = false;
					}
				}, *event
			);

			event = window.PollEvent();
		}

		window.Swap();
	}

	return 0;
}
