#pragma once

#include "Utils/AutoRelease.h"
#include "Utils/DataBuffer.h"
#include "OpenGL.h"

namespace Game {

	class Buffer
	{
	public:
		Buffer(uint32_t size);

		void Write(DataBufferView data, size_t offset) const;

		GLuint GetNativeHandle() const;

	private:
		AutoRelease<::GLuint> m_Buffer;
		uint32_t m_Size;
	};

}
