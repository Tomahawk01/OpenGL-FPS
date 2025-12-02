#include "Game/config.h"
#include "Graphics/Window.h"
#include "Graphics/Shader.h"
#include "Graphics/Program.h"
#include "Graphics/VertexData.h"
#include "Graphics/Buffer.h"
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
	};

	layout(binding = 0, std430) readonly buffer vertices
	{
		VertexData data[];
	};

	vec3 get_position(int index)
	{
		return vec3(data[index].position[0], data[index].position[1], data[index].position[2]);
	}

	void main()
	{
		gl_Position = vec4(get_position(gl_VertexID), 1.0);
	}
	)"sv;

	constexpr auto sampleFragmentShader = R"(
	#version 460 core
	
	layout(location = 0) out vec4 color;

	void main()
	{
		color = vec4(0.0, 0.5, 1.0, 1.0);
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

	const Game::VertexData triangle[] = {
		{{0.0f, 0.5f, 0.0f}},
		{{-0.5f, -0.5f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}}
	};

	const auto triangleView = Game::DataBufferView{ reinterpret_cast<const std::byte*>(triangle), sizeof(triangle) };
	const auto triangleBuffer = Game::Buffer{ sizeof(triangle) };
	triangleBuffer.Write(triangleView, std::size_t{ 0 });

	auto dummyVAO = Game::AutoRelease<GLuint>{ 0u, [](auto e) { glDeleteVertexArrays(1, &e); } };
	glGenVertexArrays(1, &dummyVAO);

	glBindVertexArray(dummyVAO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, triangleBuffer.GetNativeHandle());
	sampleProg.Use();

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

		glDrawArrays(GL_TRIANGLES, 0, 3);

		window.Swap();
	}

	return 0;
}
