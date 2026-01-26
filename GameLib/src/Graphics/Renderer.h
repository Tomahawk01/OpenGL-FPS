#pragma once

#include "Core/Scene.h"
#include "Resources/ResourceLoader.h"
#include "FrameBuffer.h"
#include "TextureManager.h"
#include "MeshManager.h"
#include "CommandBuffer.h"
#include "Program.h"
#include "Sampler.h"
#include "Window.h"
#include "OpenGL.h"
#include "Utils/AutoRelease.h"

namespace Game {

	struct RenderTarget
	{
		FrameBuffer fb;
		uint32_t colorAttachmentCount;
		uint32_t firstColorAttachmentIndex;
		uint32_t depthAttachmentIndex;
	};

	class Renderer
	{
	public:
		Renderer(const Window& window, ResourceLoader& resourceLoader, TextureManager& textureManager, MeshManager& meshManager);
		virtual ~Renderer() = default;

		void Render(Scene& scene);

	protected:
		virtual void PostRender(Scene& scene);

		const Window& m_Window;
		AutoRelease<GLuint> m_DummyVAO;
		CommandBuffer m_CommandBuffer;
		CommandBuffer m_PostProcessingCommandBuffer;
		Entity m_PostProcessSprite;
		MultiBuffer<PersistentBuffer> m_CameraBuffer;
		MultiBuffer<PersistentBuffer> m_LightBuffer;
		MultiBuffer<PersistentBuffer> m_ObjectDataBuffer;
		Program m_GBufferProgram;
		Program m_LightPassProgram;
		Sampler m_FBSampler;
		RenderTarget m_GBufferRT;
		RenderTarget m_LightPassRT;
	};

}
