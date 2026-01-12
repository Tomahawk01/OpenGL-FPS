#include "config.h"
#include "Core/Scene.h"
#include "Resources/FileResourceLoader.h"
#include "Resources/EmbeddedResourceLoader.h"
#include "Graphics/Window.h"
#include "Graphics/Shader.h"
#include "Graphics/Renderer.h"
#include "Graphics/MeshData.h"
#include "Graphics/DebugUI.h"
#include "Utils/Formatter.h"
#include "Utils/Log.h"
#include "Utils/SystemInfo.h"

#include <numbers>
#include <memory>
#include <unordered_map>

namespace {

	template<class ...Args>
	std::vector<Game::VertexData> Vertices(Args&&... args)
	{
		return std::views::zip_transform(
			[]<class ...A>(A&&... a) { return Game::VertexData{ std::forward<A>(a)... }; },
			std::forward<Args>(args)...
		) | std::ranges::to<std::vector>();
	}

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

		const Game::vec3 normals[] = {
			{0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, 1.0f},
			{0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
			{0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f},
			{-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
			{1.0f, 0.0f, 0.0f},  {1.0f, 0.0f, 0.0f},  {1.0f, 0.0f, 0.0f},
			{1.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {0.0f, 1.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {0.0f, -1.0f, 0.0f},
			{0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}
		};

		const Game::UV uvs[] = {
			{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
			{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
			{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
		};

		const std::vector<uint32_t> indices = {
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4,
			8, 9, 10, 10, 11, 8,
			12, 13, 14, 14, 15, 12,
			16, 17, 18, 18, 19, 16,
			20, 21, 22, 22, 23, 20
		};

		auto vs = Game::MeshData{ .vertices = Vertices(positions, normals, normals, normals, uvs), .indices = std::move(indices) };

		for (const auto& indices : std::views::chunk(vs.indices, 3))
		{
			auto& v0 = vs.vertices[indices[0]];
			auto& v1 = vs.vertices[indices[1]];
			auto& v2 = vs.vertices[indices[2]];

			const auto edge1 = v1.position - v0.position;
			const auto edge2 = v2.position - v0.position;

			const auto deltaUV1 = Game::UV{ .s = v1.uv.s - v0.uv.s, .t = v1.uv.t - v0.uv.t };
			const auto deltaUV2 = Game::UV{ .s = v2.uv.s - v0.uv.s, .t = v2.uv.t - v0.uv.t };

			const auto f = 1.0f / (deltaUV1.s * deltaUV2.t - deltaUV2.s * deltaUV1.t);

			const auto tangent = Game::vec3{
				f * (deltaUV2.t * edge1.x - deltaUV1.t * edge2.x),
				f * (deltaUV2.t * edge1.y - deltaUV1.t * edge2.y),
				f * (deltaUV2.t * edge1.z - deltaUV1.t * edge2.z),
			};

			v0.tangent += tangent;
			v1.tangent += tangent;
			v2.tangent += tangent;
		}

		for (auto& v : vs.vertices)
		{
			v.tangent = Game::vec3::Normalize(v.tangent - v.normal * Game::vec3::Dot(v.normal, v.tangent));
			v.bitangent = Game::vec3::Normalize(Game::vec3::Cross(v.normal, v.tangent));
		}

		return vs;
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

	std::unique_ptr<Game::ResourceLoader> resourceLoader = std::make_unique<Game::EmbeddedResourceLoader>();

	const auto diamondFloorAlbedoData = resourceLoader->LoadDataBuffer("textures\\diamond_floor_albedo.png");
	const auto diamondFloorAlbedo = Game::LoadTexture(diamondFloorAlbedoData);
	const auto sampler = Game::Sampler{ Game::FilterType::LINEAR, Game::FilterType::LINEAR, "simple_sampler" };
	const auto diamondFloorAlbedoTexture = Game::Texture{ diamondFloorAlbedo, "diamond_floor_albedo", sampler };

	const auto diamondFloorNormalData = resourceLoader->LoadDataBuffer("textures\\diamond_floor_normal.png");
	const auto diamondFloorNormal = Game::LoadTexture(diamondFloorNormalData);
	const auto diamondFloorNormalTexture = Game::Texture{ diamondFloorNormal, "diamond_floor_normal", sampler };

	const auto diamondFloorSpecularData = resourceLoader->LoadDataBuffer("textures\\diamond_floor_specular.png");
	const auto diamondFloorSpecular = Game::LoadTexture(diamondFloorSpecularData);
	const auto diamondFloorSpecularTexture = Game::Texture{ diamondFloorSpecular, "diamond_floor_specular", sampler };

	auto meshManager = Game::MeshManager{};
	auto materialManager = Game::MaterialManager{};
	auto renderer = Game::Renderer{ *resourceLoader };
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
		},
		.theOneTexture = diamondFloorAlbedoTexture,
		.theOneNormal = diamondFloorNormalTexture,
		.theOneSpecular = diamondFloorSpecularTexture,
		.lights = {
			.ambient = {0.5f, 0.5f, 0.5f},
			.light = {
				.position = {},
				.color = { 1.0f, 1.0f, 1.0f },
				.constantAttenuation = 1.0f,
				.linearAttenuation = 0.007f,
				.quadraticAttenuation = 0.0002f,
				.specularPower = 32.0f
			}
		}
	};

	scene.entities.push_back(
		{ .name = "cube1", .meshView = meshManager.Load(Cube()), .transform = {{10.0f, 0.0f, -10.0f}, {5.0f}, {}}, .materialKey = materialKeyRed }
	);

	scene.entities.push_back(
		{ .name = "cube2", .meshView = meshManager.Load(Cube()), .transform = {{-10.0f, 0.0f, -10.0f}, {5.0f}, {}}, .materialKey = materialKeyGreen }
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
