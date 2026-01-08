#include "Renderer.h"

#include "ObjectData.h"
#include "Utils.h"

#include <string_view>
#include <ranges>
#include <span>

using namespace std::literals;

namespace {

	constexpr const char sampleVertexShader[] = {
		#embed "../Game/assets/shaders/simple.vert" suffix(, )
		0
	};

	constexpr const char sampleFragmentShader[] = {
		#embed "../Game/assets/shaders/simple.frag" suffix(, )
		0
	};

	Game::Program CreateProgram()
	{
		const auto sampleVert = Game::Shader{ sampleVertexShader, Game::ShaderType::VERTEX, "sample_vertex_shader"sv };
		const auto sampleFrag = Game::Shader{ sampleFragmentShader, Game::ShaderType::FRAGMENT, "sample_fragment_shader"sv };
		return { sampleVert, sampleFrag, "sample_prog"sv };
	}

}

namespace Game {

	Renderer::Renderer()
		: m_DummyVAO{ 0u, [](auto e) { glDeleteVertexArrays(1, &e); } }
		, m_CommandBuffer{}
		, m_CameraBuffer{ sizeof(CameraData), "camera_buffer" }
		, m_LightBuffer{ sizeof(LightData), "light_buffer" }
		, m_ObjectDataBuffer{ sizeof(ObjectData), "object_data_buffer" }
		, m_Program{ CreateProgram() }
	{
		glGenVertexArrays(1, &m_DummyVAO);
		glBindVertexArray(m_DummyVAO);

		m_Program.Use();
	}

	void Renderer::Render(const Scene& scene)
	{
		m_CameraBuffer.Write(scene.camera.GetDataView(), 0);

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
		m_ObjectDataBuffer.Write(std::as_bytes(std::span{ objectData.data(), objectData.size() }), 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_ObjectDataBuffer.GetNativeHandle());

		scene.materialManager.Sync();
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, scene.materialManager.GetNativeHandle());

		m_LightBuffer.Write(std::as_bytes(std::span<const LightData, 1>{&scene.lights, 1}), 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_LightBuffer.GetNativeHandle());

		glProgramUniformHandleui64ARB(m_Program.GetNativeHandle(), 0, scene.theOneTexture.GetNativeHandle());
		glProgramUniformHandleui64ARB(m_Program.GetNativeHandle(), 1, scene.theOneNormal.GetNativeHandle());
		glProgramUniformHandleui64ARB(m_Program.GetNativeHandle(), 2, scene.theOneSpecular.GetNativeHandle());

		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, reinterpret_cast<const void*>(m_CommandBuffer.OffsetBytes()), commandCount, 0);

		m_CommandBuffer.Advance();
		m_CameraBuffer.Advance();
		m_LightBuffer.Advance();
		m_ObjectDataBuffer.Advance();
		scene.materialManager.Advance();
	}

}
