#include "Game/config.h"
#include "Graphics/Window.h"
#include "Graphics/Shader.h"
#include "Graphics/Scene.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/MeshManager.h"
#include "Graphics/Renderer.h"
#include "Utils/Formatter.h"
#include "Utils/Log.h"
#include "Utils/SystemInfo.h"

int main()
{
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	Game::Log::Info("Game version: {}.{}.{}", Game::Version::MAJOR, Game::Version::MINOR, Game::Version::PATCH);
	Game::Log::Info("{}", Game::GetSystemInfo());

	auto window = Game::Window{ Game::WindowMode::WINDOWED, 1920u, 1080u, 0u, 0u };
	auto running = true;

	auto meshManager = Game::MeshManager{};
	auto renderer = Game::Renderer{};

	auto scene = Game::Scene{ .entities = {}, .meshManager = meshManager };

	scene.entities.push_back({ meshManager.Load({
		{{0.0f, 0.0f, 0.0f}, Game::Colors::Azure},
		{{-0.5f, 0.0f, 0.0f}, Game::Color{0.6f, 0.1f, 0.0f}},
		{{-0.5f, 0.5f, 0.0f}, Game::Color{0.42f, 0.42f, 0.42f}} })
	});

	while (running)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto event = window.PollEvent();
		while (event && running)
		{
			std::visit(
				[&](auto&& arg)
				{
					using T = std::decay_t<decltype(arg)>;

					if constexpr (std::same_as<T, Game::KeyEvent>)
					{
						if (arg.GetKey() == Game::Key::T)
						{
							scene.entities.push_back({ meshManager.Load({
								{{0.0f, 0.0f, 0.0f}, Game::Colors::Azure},
								{{-0.5f, 0.5f, 0.0f}, Game::Color{0.42f, 0.42f, 0.42f}},
								{{0.0f, 0.5f, 0.0f}, Game::Color{0.6f, 0.1f, 0.0f}} })
							});
						}
						else
						{
							Game::Log::Info("Stopping...");
							running = false;
						}
					}
				}, *event
			);

			event = window.PollEvent();
		}

		renderer.Render(scene);

		window.Swap();
	}

	return 0;
}
