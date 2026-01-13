#include "Renderer.h"

#include "ObjectData.h"
#include "Utils.h"

#include <string_view>
#include <ranges>
#include <span>

using namespace std::literals;

namespace {

	Game::Program CreateProgram(Game::ResourceLoader& resourceLoader)
	{
		const auto sampleVert = Game::Shader{ resourceLoader._LoadString("shaders\\simple.vert"), Game::ShaderType::VERTEX, "sample_vertex_shader"sv };
		const auto sampleFrag = Game::Shader{ resourceLoader._LoadString("shaders\\simple.frag"), Game::ShaderType::FRAGMENT, "sample_fragment_shader"sv };
		return { sampleVert, sampleFrag, "sample_prog"sv };
	}

}

namespace Game {

	Renderer::Renderer(ResourceLoader& resourceLoader)
		: m_DummyVAO{ 0u, [](auto e) { glDeleteVertexArrays(1, &e); } }
		, m_CommandBuffer{}
		, m_CameraBuffer{ sizeof(CameraData), "camera_buffer" }
		, m_LightBuffer{ sizeof(LightData), "light_buffer" }
		, m_ObjectDataBuffer{ sizeof(ObjectData), "object_data_buffer" }
		, m_Program{ CreateProgram(resourceLoader) }
	{
		glGenVertexArrays(1, &m_DummyVAO);
		glBindVertexArray(m_DummyVAO);

		m_Program.Use();
	}

	void Renderer::Render(const Scene& scene)
	{
		m_CameraBuffer.Write(scene.camera.GetDataView(), 0zu);

		const auto [vertexBufferHandle, indexBufferHandle] = scene.meshManager.GetNativeHandle();
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexBufferHandle);
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, m_CameraBuffer.GetNativeHandle(), m_CameraBuffer.FrameOffsetBytes(), sizeof(CameraData));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle);

		const auto commandCount = m_CommandBuffer.Build(scene);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_CommandBuffer.GetNativeHandle());

		const auto objectData = scene.entities |
								std::views::transform([&scene](const auto& e)
													  {
														  const auto index = scene.materialManager.Index(e.materialKey);
														  return ObjectData{ .model = e.transform, .materialIDIndex = index, .padding = {} };
													  }) |
								std::ranges::to<std::vector>();
		ResizeGPUBuffer(objectData, m_ObjectDataBuffer, "object_data_buffer");
		m_ObjectDataBuffer.Write(std::as_bytes(std::span{ objectData.data(), objectData.size() }), 0zu);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_ObjectDataBuffer.GetNativeHandle());

		scene.materialManager.Sync();
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, scene.materialManager.GetNativeHandle());

		m_LightBuffer.Write(std::as_bytes(std::span<const LightData, 1zu>{&scene.lights, 1zu}), 0zu);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_LightBuffer.GetNativeHandle());

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, scene.textureManager.GetNativeHandle());

		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, reinterpret_cast<const void*>(m_CommandBuffer.OffsetBytes()), commandCount, 0);

		m_CommandBuffer.Advance();
		m_CameraBuffer.Advance();
		m_LightBuffer.Advance();
		m_ObjectDataBuffer.Advance();
		scene.materialManager.Advance();
	}

}
