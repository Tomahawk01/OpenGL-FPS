#include "Buffer.h"

#include "Utils/Error.h"

namespace Game {

	Buffer::Buffer(size_t size, std::string_view name)
		: m_Buffer{ 0u, [](auto buffer) { glDeleteBuffers(1, &buffer); } }
		, m_Size{ size }
	{
		glCreateBuffers(1, &m_Buffer);
		glNamedBufferStorage(m_Buffer, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
		glObjectLabel(GL_BUFFER, m_Buffer, name.length(), name.data());
	}

	void Buffer::Write(DataBufferView data, size_t offset) const
	{
		Expect(m_Size >= data.size_bytes() + offset, "Buffer is too small");
		glNamedBufferSubData(m_Buffer, offset, data.size(), data.data());
	}

	GLuint Buffer::GetNativeHandle() const
	{
		return m_Buffer;
	}

	size_t Buffer::GetSize() const
	{
		return m_Size;
	}

}
