#pragma once

#include "Core/Scene.h"
#include "Resources/ResourceLoader.h"
#include "FrameBuffer.h"
#include "TextureManager.h"
#include "MeshManager.h"
#include "CommandBuffer.h"
#include "Program.h"
#include "Sampler.h"
#include "OpenGL.h"
#include "Utils/AutoRelease.h"

namespace Game {

	class Renderer
	{
	public:
		Renderer(uint32_t width, uint32_t height, ResourceLoader& resourceLoader, TextureManager& textureManager, MeshManager& meshManager);

		void Render(const Scene& scene);

	private:
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
		uint32_t m_FBTextureIndex;
		FrameBuffer m_FB;
		uint32_t m_LightPassTextureIndex;
		FrameBuffer m_LightPassFB;
	};

}
