#pragma once

#include "CommandBuffer.h"
#include "Scene.h"
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
		Program m_Program;
	};

}
