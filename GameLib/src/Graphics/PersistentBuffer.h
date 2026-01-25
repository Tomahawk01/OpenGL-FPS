#pragma once

#include "Utils/AutoRelease.h"
#include "Utils/DataBuffer.h"
#include "OpenGL.h"

#include <string>
#include <string_view>

namespace Game {

	class PersistentBuffer
	{
	public:
		PersistentBuffer(size_t size, std::string_view name);

		void Write(DataBufferView data, size_t offset) const;

		GLuint GetNativeHandle() const;
		std::string_view GetName() const;

		std::string to_string() const;

	private:
		AutoRelease<GLuint> m_Buffer;
		size_t m_Size;
		void* m_Map;
		std::string m_Name;
	};

}
