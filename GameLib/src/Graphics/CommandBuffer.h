#pragma once

#include "Scene.h"
#include "MultiBuffer.h"
#include "PersistentBuffer.h"
#include "OpenGL.h"

#include <cstdint>
#include <string>

namespace Game {

	class CommandBuffer
	{
	public:
		CommandBuffer();

		uint32_t Build(const Scene& scene);
		void Advance();

		GLuint GetNativeHandle() const;
		std::string to_string() const;

	private:
		MultiBuffer<PersistentBuffer> m_CommandBuffer;
	};

}
