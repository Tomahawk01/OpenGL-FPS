#include "PersistentBuffer.h"

#include "Utils/Error.h"

#include <cstring>

namespace Game {

	PersistentBuffer::PersistentBuffer(size_t size, std::string_view name)
		: m_Buffer{ 0u, [](auto buffer) { glUnmapNamedBuffer(buffer); glDeleteBuffers(1, &buffer); } }
		, m_Size{ size }
	{
		const auto flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		glCreateBuffers(1, &m_Buffer);
		glNamedBufferStorage(m_Buffer, size, nullptr, GL_DYNAMIC_STORAGE_BIT | flags);
		glObjectLabel(GL_BUFFER, m_Buffer, name.length(), name.data());

		m_Map = glMapNamedBufferRange(m_Buffer, 0, size, flags);
	}

	void PersistentBuffer::Write(DataBufferView data, size_t offset) const
	{
		Expect(m_Size >= data.size_bytes() + offset, "Buffer is too small");
		std::memcpy(reinterpret_cast<std::byte*>(m_Map) + offset, data.data(), data.size_bytes());
	}

	GLuint PersistentBuffer::GetNativeHandle() const
	{
		return m_Buffer;
	}

}
