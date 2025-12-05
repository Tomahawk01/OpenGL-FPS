#pragma once

#include "Utils/AutoRelease.h"
#include "Utils/DataBuffer.h"
#include "OpenGL.h"

#include <string_view>

namespace Game {

	class PersistentBuffer
	{
	public:
		PersistentBuffer(size_t size, std::string_view name);

		void Write(DataBufferView data, size_t offset) const;

		GLuint GetNativeHandle() const;

	private:
		AutoRelease<GLuint> m_Buffer;
		size_t m_Size;
		void* m_Map;
	};

}
