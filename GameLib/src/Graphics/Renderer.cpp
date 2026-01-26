#include "Renderer.h"

#include "ObjectData.h"
#include "Utils.h"

#include <string_view>
#include <ranges>
#include <span>

using namespace std::literals;

namespace {

	Game::Program CreateProgram(Game::ResourceLoader& resourceLoader, std::string_view vertexPath, std::string_view vertexName, std::string_view fragmentPath, std::string_view fragmentName, std::string_view programName)
	{
		const auto simpleVert = Game::Shader{ resourceLoader._LoadString(vertexPath), Game::ShaderType::VERTEX, vertexName };
		const auto simpleFrag = Game::Shader{ resourceLoader._LoadString(fragmentPath), Game::ShaderType::FRAGMENT, fragmentName };
		return { simpleVert, simpleFrag, programName };
	}

	Game::RenderTarget CreateRenderTarget(uint32_t colorAttachmentCount, uint32_t width, uint32_t height, Game::Sampler& sampler, Game::TextureManager& textureManager, std::string_view name)
	{
		const auto colorAttachmentTextureData = Game::TextureData{
			.width = width,
			.height = height,
			.format = Game::TextureFormat::RGB16F,
			.data = std::nullopt
		};

		auto colorAttachments = std::views::iota(0u, colorAttachmentCount) |
			std::views::transform([&](auto index) { return Game::Texture{ colorAttachmentTextureData, std::format("{}_{}_texture", name, index), sampler }; }) |
			std::ranges::to<std::vector>();

		const auto firstIndex = textureManager.Add(std::move(colorAttachments));

		const auto depthTextureData = Game::TextureData{
			.width = width,
			.height = height,
			.format = Game::TextureFormat::DEPTH24,
			.data = std::nullopt
		};
		auto depthTexture = Game::Texture{ depthTextureData, std::format("{}_depth_texture", name), sampler };
		const auto depthTextureIndex = textureManager.Add(std::move(depthTexture));

		auto fb = Game::FrameBuffer{
			textureManager.GetTextures(std::views::iota(firstIndex, firstIndex + colorAttachmentCount) | std::ranges::to<std::vector>()),
			textureManager.GetTexture(depthTextureIndex),
			std::format("{}_frame_buffer", name)
		};

		return {
			.fb = std::move(fb),
			.colorAttachmentCount = colorAttachmentCount,
			.firstColorAttachmentIndex = firstIndex,
			.depthAttachmentIndex = depthTextureIndex
		};
	}

	Game::MeshData Sprite()
	{
		const Game::vec3 positions[] = {
			{-1.0f, 1.0f, 0.0f},
			{-1.0f, -1.0f, 0.0f},
			{1.0f, -1.0f, 0.0f},
			{1.0f, 1.0f, 0.0f}
		};

		const Game::UV uvs[] = {
			{0.0f, 1.0f},
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f}
		};

		const std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };

		return {
			Vertices(positions, positions, positions, positions, uvs),
			std::move(indices)
		};
	}

}

namespace Game {

	Renderer::Renderer(const Window& window, ResourceLoader& resourceLoader, TextureManager& textureManager, MeshManager& meshManager)
		: m_Window{ window }
		, m_DummyVAO{ 0u, [](auto e) { glDeleteVertexArrays(1, &e); } }
		, m_CommandBuffer{ "gbuffer_command_buffer" }
		, m_PostProcessingCommandBuffer{ "post_processing_command_buffer" }
		, m_PostProcessSprite{ "post_process_sprite", meshManager.Load(Sprite()), {}, 0u }
		, m_CameraBuffer{ sizeof(CameraData), "camera_buffer" }
		, m_LightBuffer{ sizeof(LightData), "light_buffer" }
		, m_ObjectDataBuffer{ sizeof(ObjectData), "object_data_buffer" }
		, m_GBufferProgram{ CreateProgram(resourceLoader, "shaders\\gbuffer.vert", "gbuffer_vertex_shader", "shaders\\gbuffer.frag", "gbuffer_fragment_shader", "gbuffer_prog")}
		, m_LightPassProgram{ CreateProgram(resourceLoader, "shaders\\light_pass.vert", "light_pass_vertex_shader", "shaders\\light_pass.frag", "light_pass_fragment_shader", "light_pass_prog")}
		, m_FBSampler{ FilterType::LINEAR, FilterType::LINEAR, "fb_sampler" }
		, m_GBufferRT{ CreateRenderTarget(4u, m_Window.GetRenderWidth(), m_Window.GetRenderHeight(), m_FBSampler, textureManager, "gbuffer")}
		, m_LightPassRT{ CreateRenderTarget(1u, m_Window.GetRenderWidth(), m_Window.GetRenderHeight(), m_FBSampler, textureManager, "light_pass") }
	{
		m_PostProcessingCommandBuffer.Build(m_PostProcessSprite);

		glGenVertexArrays(1, &m_DummyVAO);
		glBindVertexArray(m_DummyVAO);
	}

	void Renderer::Render(Scene& scene)
	{
		m_GBufferRT.fb.Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_GBufferProgram.Use();

		m_CameraBuffer.Write(scene.camera.GetDataView(), 0zu);

		const auto [vertexBufferHandle, indexBufferHandle] = scene.meshManager.GetNativeHandle();
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexBufferHandle);
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, m_CameraBuffer.GetNativeHandle(), m_CameraBuffer.FrameOffsetBytes(), sizeof(CameraData));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle);

		const auto commandCount = m_CommandBuffer.Build(scene);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_CommandBuffer.GetNativeHandle());

		const auto objectData = scene.entities |
								std::views::transform([](const auto& e)
													  {
														  return ObjectData{ .model = e.transform, .materialIDIndex = e.materialIndex, .padding = {} };
													  }) |
								std::ranges::to<std::vector>();
		ResizeGPUBuffer(objectData, m_ObjectDataBuffer);
		m_ObjectDataBuffer.Write(std::as_bytes(std::span{ objectData.data(), objectData.size() }), 0zu);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_ObjectDataBuffer.GetNativeHandle());

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, scene.materialManager.GetNativeHandle());

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, scene.textureManager.GetNativeHandle());

		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, reinterpret_cast<const void*>(m_CommandBuffer.OffsetBytes()), commandCount, 0);

		m_LightBuffer.Write(std::as_bytes(std::span<const LightData, 1zu>{&scene.lights, 1zu}), 0zu);

		m_LightPassRT.fb.Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_LightPassProgram.Use();
		glProgramUniform1ui(m_LightPassProgram.GetNativeHandle(), 0u, m_GBufferRT.firstColorAttachmentIndex + 0u);
		glProgramUniform1ui(m_LightPassProgram.GetNativeHandle(), 1u, m_GBufferRT.firstColorAttachmentIndex + 1u);
		glProgramUniform1ui(m_LightPassProgram.GetNativeHandle(), 2u, m_GBufferRT.firstColorAttachmentIndex + 2u);
		glProgramUniform1ui(m_LightPassProgram.GetNativeHandle(), 3u, m_GBufferRT.firstColorAttachmentIndex + 3u);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexBufferHandle);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scene.textureManager.GetNativeHandle());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_LightBuffer.GetNativeHandle());
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 3, m_CameraBuffer.GetNativeHandle(), m_CameraBuffer.FrameOffsetBytes(), sizeof(CameraData));
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_PostProcessingCommandBuffer.GetNativeHandle());
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, reinterpret_cast<const void*>(m_PostProcessingCommandBuffer.OffsetBytes()), 1u, 0);

		m_CommandBuffer.Advance();
		m_CameraBuffer.Advance();
		m_LightBuffer.Advance();
		m_ObjectDataBuffer.Advance();

		PostRender(scene);
	}

	void Renderer::PostRender(Scene&)
	{
		m_LightPassRT.fb.UnBind();

		glBlitNamedFramebuffer(
			m_LightPassRT.fb.GetNativeHandle(),
			0u,
			0u,
			0u,
			m_LightPassRT.fb.GetWidth(),
			m_LightPassRT.fb.GetHeight(),
			0u,
			0u,
			m_LightPassRT.fb.GetWidth(),
			m_LightPassRT.fb.GetHeight(),
			GL_COLOR_BUFFER_BIT,
			GL_NEAREST);
	}

}
