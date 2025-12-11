#include "config.h"
#include "Graphics/Window.h"
#include "Graphics/Shader.h"
#include "Graphics/Scene.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/MeshManager.h"
#include "Graphics/Renderer.h"
#include "Graphics/MeshData.h"
#include "Utils/Formatter.h"
#include "Utils/Log.h"
#include "Utils/SystemInfo.h"

#include <numbers>

namespace {

	Game::MeshData Cube()
	{
		const Game::vec3 positions[] = {
			{-1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f},
			{-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, -1.0f}, {-1.0f, 1.0f, -1.0f},
			{-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, -1.0f},
			{1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, -1.0f},
			{-1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, -1.0f}, {-1.0f, 1.0f, -1.0f},
			{-1.0f, -1.0f, 1.0f}, {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, 1.0f}
		};

		const std::vector<uint32_t> indices = {
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4,
			8, 9, 10, 10, 11, 8,
			12, 13, 14, 14, 15, 12,
			16, 17, 18, 18, 19, 16,
			20, 21, 22, 22, 23, 20
		};

		return { .vertices = positions | std::views::transform([](const auto& e)
												 {
													 return Game::VertexData{.position = e, .color = Game::Colors::Azure };
												 }) | std::ranges::to<std::vector>(),
				 .indices = std::move(indices)
		};
	}

}

int main()
{
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	Game::Log::Info("Game version: {}.{}.{}", Game::Version::MAJOR, Game::Version::MINOR, Game::Version::PATCH);
	Game::Log::Info("{}", Game::GetSystemInfo());

	auto window = Game::Window{ Game::WindowMode::WINDOWED, 1920u, 1080u, 0u, 0u };
	auto running = true;

	auto meshManager = Game::MeshManager{};
	auto renderer = Game::Renderer{};

	auto scene = Game::Scene{
		.entities = {},
		.meshManager = meshManager,
		.camera = {
			{},
			{0.0f, 0.0f, -1.0f},
			{0.0f, 1.0f, 0.0f},
			std::numbers::pi_v<float> / 4.0f,
			static_cast<float>(window.GetRenderHeight()), static_cast<float>(window.GetRenderHeight()),
			0.1f, 1000.0f
		}
	};

	scene.entities.push_back(
		{ meshManager.Load(Cube()) }
	);

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
						if (arg.GetKey() == Game::Key::ESC)
						{
							Game::Log::Info("Stopping...");
							running = false;
						}
					}
					else if constexpr (std::same_as<T, Game::MouseEvent>)
					{
						static constexpr auto sensitivity = 0.002f;
						const auto deltaX = arg.GetDeltaX() * sensitivity;
						const auto deltaY = arg.GetDeltaY() * sensitivity;
						scene.camera.AddYaw(deltaX);
						scene.camera.AddPitch(-deltaY);
					}
				}, *event
			);

			event = window.PollEvent();
		}

		scene.camera.Translate({ 0.0f, 0.0f, 0.01f });

		renderer.Render(scene);

		window.Swap();
	}

	return 0;
}
