#include "Game/config.h"
#include "Graphics/Window.h"
#include "Graphics/Shader.h"
#include "Graphics/Program.h"
#include "Graphics/Buffer.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/MeshManager.h"
#include "Graphics/Scene.h"
#include "Utils/Formatter.h"
#include "Utils/Log.h"
#include "Utils/SystemInfo.h"

using namespace std::literals;

namespace {

	constexpr auto sampleVertexShader = R"(
	#version 460 core
	
	struct VertexData
	{
		float position[3];
		float color[3];
	};

	layout(binding = 0, std430) readonly buffer vertices
	{
		VertexData data[];
	};

	vec3 get_position(int index)
	{
		return vec3(data[index].position[0], data[index].position[1], data[index].position[2]);
	}

	vec3 get_color(int index)
	{
		return vec3(data[index].color[0], data[index].color[1], data[index].color[2]);
	}

	layout(location = 0) out vec3 out_color;

	void main()
	{
		gl_Position = vec4(get_position(gl_VertexID), 1.0);
		out_color = get_color(gl_VertexID);
	}
	)"sv;

	constexpr auto sampleFragmentShader = R"(
	#version 460 core
	
	layout(location = 0) in vec3 in_color;

	layout(location = 0) out vec4 out_color;

	void main()
	{
		out_color = vec4(in_color, 1.0);
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

	const auto sampleVert = Game::Shader{ sampleVertexShader, Game::ShaderType::VERTEX, "sample_vertex_shader"sv };
	const auto sampleFrag = Game::Shader{ sampleFragmentShader, Game::ShaderType::FRAGMENT, "sample_fragment_shader"sv };
	const auto sampleProg = Game::Program{ sampleVert, sampleFrag, "sample_prog"sv };

	auto meshManager = Game::MeshManager{};
	auto commandBuffer = Game::CommandBuffer{};

	auto scene = Game::Scene{ .entities = {}, .meshManager = meshManager };

	scene.entities.push_back({ meshManager.Load({
		{{0.0f, 0.0f, 0.0f}, Game::Colors::Azure},
		{{-0.5f, 0.0f, 0.0f}, Game::Color{0.6f, 0.1f, 0.0f}},
		{{-0.5f, 0.5f, 0.0f}, Game::Color{0.42f, 0.42f, 0.42f}} })
	});

	scene.entities.push_back({ meshManager.Load({
		{{0.0f, 0.0f, 0.0f}, Game::Colors::Azure},
		{{-0.5f, 0.5f, 0.0f}, Game::Color{0.42f, 0.42f, 0.42f}},
		{{0.0f, 0.5f, 0.0f}, Game::Color{0.6f, 0.1f, 0.0f}} })
	});

	auto dummyVAO = Game::AutoRelease<GLuint>{ 0u, [](auto e) { glDeleteVertexArrays(1, &e); } };
	glGenVertexArrays(1, &dummyVAO);

	glBindVertexArray(dummyVAO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, meshManager.GetNativeHandle());
	sampleProg.Use();

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
						Game::Log::Info("Stopping...");
						running = false;
					}
				}, *event
			);

			event = window.PollEvent();
		}

		const auto commandCount = commandBuffer.Build(scene);

		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer.GetNativeHandle());

		glMultiDrawArraysIndirect(GL_TRIANGLES, nullptr, commandCount, 0);

		commandBuffer.Advance();

		window.Swap();
	}

	return 0;
}
