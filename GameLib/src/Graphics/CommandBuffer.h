#pragma once

#include "Core/Scene.h"
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
		uint32_t Build(const Entity& entity);
		void Advance();
		size_t OffsetBytes() const;

		GLuint GetNativeHandle() const;
		std::string to_string() const;

	private:
		MultiBuffer<PersistentBuffer> m_CommandBuffer;
	};

}
