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

	Game::FrameBuffer CreateFrameBuffer(uint32_t width, uint32_t height, Game::Sampler& sampler, Game::TextureManager& textureManager, uint32_t& fbTextureIndex)
	{
		const auto fbTextureData = Game::TextureData{
			.width = width,
			.height = height,
			.format = Game::TextureFormat::RGB16F,
			.data = std::nullopt
		};
		auto fbTexture = Game::Texture{ fbTextureData, "fb_texture", sampler };
		fbTextureIndex = textureManager.Add(std::move(fbTexture));

		const auto depthTextureData = Game::TextureData{
			.width = width,
			.height = height,
			.format = Game::TextureFormat::DEPTH24,
			.data = std::nullopt
		};
		auto depthTexture = Game::Texture{ depthTextureData, "depth_texture", sampler };
		const auto depthTextureIndex = textureManager.Add(std::move(depthTexture));

		return {
			textureManager.GetTextures({fbTextureIndex}),
			textureManager.GetTexture(depthTextureIndex),
			"main_frame_buffer"
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

	Renderer::Renderer(uint32_t width, uint32_t height, ResourceLoader& resourceLoader, TextureManager& textureManager, MeshManager& meshManager)
		: m_DummyVAO{ 0u, [](auto e) { glDeleteVertexArrays(1, &e); } }
		, m_CommandBuffer{}
		, m_PostProcessingCommandBuffer{}
		, m_PostProcessSprite{ "post_process_sprite", meshManager.Load(Sprite()), {}, {0u} }
		, m_CameraBuffer{ sizeof(CameraData), "camera_buffer" }
		, m_LightBuffer{ sizeof(LightData), "light_buffer" }
		, m_ObjectDataBuffer{ sizeof(ObjectData), "object_data_buffer" }
		, m_GBufferProgram{ CreateProgram(resourceLoader, "shaders\\simple.vert", "simple_vertex_shader", "shaders\\simple.frag", "simple_fragment_shader", "gbuffer_prog")}
		, m_LightPassProgram{ CreateProgram(resourceLoader, "shaders\\light_pass.vert", "light_pass_vertex_shader", "shaders\\light_pass.frag", "light_pass_fragment_shader", "light_pass_prog")}
		, m_FBSampler{ FilterType::LINEAR, FilterType::LINEAR, "fb_sampler" }
		, m_FBTextureIndex{}
		, m_FB{ CreateFrameBuffer(width, height, m_FBSampler, textureManager, m_FBTextureIndex) }
		, m_LightPassTextureIndex{}
		, m_LightPassFB{ CreateFrameBuffer(width, height, m_FBSampler, textureManager, m_LightPassTextureIndex) }
	{
		m_PostProcessingCommandBuffer.Build(m_PostProcessSprite);

		glGenVertexArrays(1, &m_DummyVAO);
		glBindVertexArray(m_DummyVAO);
	}

	void Renderer::Render(const Scene& scene)
	{
		m_FB.Bind();
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

		m_LightPassFB.Bind();
		m_LightPassProgram.Use();
		glClear(GL_COLOR_BUFFER_BIT);
		glProgramUniform1ui(m_LightPassProgram.GetNativeHandle(), 0u, 1u);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexBufferHandle);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scene.textureManager.GetNativeHandle());
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, reinterpret_cast<const void*>(m_PostProcessingCommandBuffer.OffsetBytes()), 1u, 0);

		m_CommandBuffer.Advance();
		m_CameraBuffer.Advance();
		m_LightBuffer.Advance();
		m_ObjectDataBuffer.Advance();
		scene.materialManager.Advance();

		m_LightPassFB.UnBind();

		glBlitNamedFramebuffer(
			m_LightPassFB.GetNativeHandle(),
			0u,
			0u,
			0u,
			m_LightPassFB.GetWidth(),
			m_LightPassFB.GetHeight(),
			0u,
			0u,
			m_LightPassFB.GetWidth(),
			m_LightPassFB.GetHeight(),
			GL_COLOR_BUFFER_BIT,
			GL_NEAREST);
	}

}
