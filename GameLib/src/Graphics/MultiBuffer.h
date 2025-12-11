#pragma once

#include "Utils/DataBuffer.h"

#include <string_view>

namespace Game {

	template<class T>
	concept IsBuffer = requires(T t, DataBufferView data, size_t offset)
	{
		{ t.Write(data, offset) };
	};

	// @brief A multi buffer wrapper over a Buffer type.
	// Will allocate size * Frames amount of data and can advance through the internal frames
	template<IsBuffer Buffer, size_t Frames = size_t{ 3 } >
	class MultiBuffer
	{
	public:
		MultiBuffer(size_t size, std::string_view name)
			: m_Buffer{ size * Frames, name }
			, m_OriginalSize{ size }
			, m_FrameOffset{}
		{}

		void Write(DataBufferView data, size_t offset)
		{
			m_Buffer.Write(data, offset + m_FrameOffset);
		}

		void Advance()
		{
			m_FrameOffset = (m_FrameOffset + m_OriginalSize) % (m_OriginalSize * Frames);
		}

		const Buffer& GetBuffer() const
		{
			return m_Buffer;
		}

		size_t GetOriginalSize() const
		{
			return m_OriginalSize;
		}

		size_t FrameOffsetBytes() const
		{
			return m_FrameOffset;
		}

	private:
		Buffer m_Buffer;
		size_t m_OriginalSize;
		size_t m_FrameOffset;
	};

}
