#pragma once

#include "CommandBuffer.h"
#include "Core/Scene.h"
#include "Program.h"
#include "OpenGL.h"
#include "Utils/AutoRelease.h"

namespace Game {

	class Renderer
	{
	public:
		Renderer();

		void Render(const Scene& scene);

	private:
		AutoRelease<GLuint> m_DummyVAO;
		CommandBuffer m_CommandBuffer;
		MultiBuffer<PersistentBuffer> m_CameraBuffer;
		MultiBuffer<PersistentBuffer> m_ObjectDataBuffer;
		Program m_Program;
	};

}
