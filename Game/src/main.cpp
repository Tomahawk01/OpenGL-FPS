#include "config.h"
#include "Graphics/Window.h"
#include "Graphics/Shader.h"
#include "Graphics/Scene.h"
#include "Graphics/Renderer.h"
#include "Graphics/MeshData.h"
#include "Graphics/DebugUI.h"
#include "Utils/Formatter.h"
#include "Utils/Log.h"
#include "Utils/SystemInfo.h"

#include <numbers>
#include <unordered_map>

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
													 return Game::VertexData{ .position = e };
												 }) | std::ranges::to<std::vector>(),
				 .indices = std::move(indices)
		};
	}

	Game::vec3 WalkDirection(std::unordered_map<Game::Key, bool>& keyState, const Game::Camera& camera)
	{
		auto direction = Game::vec3{};

		if (keyState[Game::Key::W])
		{
			direction += camera.GetDirection();
		}
		if (keyState[Game::Key::S])
		{
			direction -= camera.GetDirection();
		}
		if (keyState[Game::Key::D])
		{
			direction += camera.Right();
		}
		if (keyState[Game::Key::A])
		{
			direction -= camera.Right();
		}

		if (keyState[Game::Key::Q])
		{
			direction += camera.Up();
		}
		if (keyState[Game::Key::E])
		{
			direction -= camera.Up();
		}

		constexpr auto speed = 0.5f;
		return Game::vec3::Normalize(direction) * speed;
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
	auto materialManager = Game::MaterialManager{};
	auto renderer = Game::Renderer{};
	auto debugUI = Game::DebugUI{ window };
	auto debugMode = false;

	const auto materialKeyRed = materialManager.Add(Game::Color{ 1.0f, 0.0f, 0.0f });
	const auto materialKeyBlue = materialManager.Add(Game::Color{ 0.0f, 0.0f, 1.0f });
	const auto materialKeyGreen = materialManager.Add(Game::Color{ 0.0f, 1.0f, 0.0f });
	materialManager.Remove(materialKeyBlue);

	auto scene = Game::Scene{
		.entities = {},
		.meshManager = meshManager,
		.materialManager = materialManager,
		.camera = {
			{},
			{0.0f, 0.0f, -1.0f},
			{0.0f, 1.0f, 0.0f},
			std::numbers::pi_v<float> / 4.0f,
			static_cast<float>(window.GetRenderWidth()), static_cast<float>(window.GetRenderHeight()),
			0.1f, 1000.0f
		}
	};

	scene.entities.push_back(
		{ .meshView = meshManager.Load(Cube()), .transform = {{10.0f, 0.0f, -10.0f}, {5.0f}, {}}, .materialKey = materialKeyRed }
	);

	scene.entities.push_back(
		{ .meshView = meshManager.Load(Cube()), .transform = {{-10.0f, 0.0f, -10.0f}, {5.0f}, {}}, .materialKey = materialKeyGreen }
	);

	auto keyState = std::unordered_map<Game::Key, bool>{
		{Game::Key::W, false},
		{Game::Key::A, false},
		{Game::Key::S, false},
		{Game::Key::D, false}
	};

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
						if (arg == Game::KeyEvent{ Game::Key::F1, Game::KeyState::DOWN })
						{
							debugMode = !debugMode;
						}
						else
						{
							keyState[arg.GetKey()] = arg.GetState() == Game::KeyState::DOWN;
						}
					}
					else if constexpr (std::same_as<T, Game::MouseEvent>)
					{
						if (!debugMode)
						{
							static constexpr auto sensitivity = 0.002f;
							const auto deltaX = arg.GetDeltaX() * sensitivity;
							const auto deltaY = arg.GetDeltaY() * sensitivity;
							scene.camera.AddYaw(deltaX);
							scene.camera.AddPitch(-deltaY);
						}
					}
					else if constexpr (std::same_as<T, Game::MouseButtonEvent>)
					{
						debugUI.AddMouseEvent(arg);
					}
				}, *event
			);

			event = window.PollEvent();
		}

		scene.camera.Translate(WalkDirection(keyState, scene.camera));

		renderer.Render(scene);
		if (debugMode)
		{
			debugUI.Render(scene);
		}

		window.Swap();
	}

	return 0;
}
