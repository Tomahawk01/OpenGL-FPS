#pragma once

#include "Core/Scene.h"
#include "Resources/ResourceLoader.h"
#include "FrameBuffer.h"
#include "TextureManager.h"
#include "CommandBuffer.h"
#include "Program.h"
#include "Sampler.h"
#include "OpenGL.h"
#include "Utils/AutoRelease.h"

namespace Game {

	class Renderer
	{
	public:
		Renderer(uint32_t width, uint32_t height, ResourceLoader& resourceLoader, TextureManager& textureManager);

		void Render(const Scene& scene);

	private:
		AutoRelease<GLuint> m_DummyVAO;
		CommandBuffer m_CommandBuffer;
		MultiBuffer<PersistentBuffer> m_CameraBuffer;
		MultiBuffer<PersistentBuffer> m_LightBuffer;
		MultiBuffer<PersistentBuffer> m_ObjectDataBuffer;
		Program m_Program;
		Sampler m_FBSampler;
		FrameBuffer m_FB;
	};

}
