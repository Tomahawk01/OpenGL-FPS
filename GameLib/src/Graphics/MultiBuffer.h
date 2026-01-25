#pragma once

#include "Utils.h"
#include "Utils/DataBuffer.h"

#include <string_view>

namespace Game {

	// @brief A multi buffer wrapper over a Buffer type.
	// Will allocate size * Frames amount of data and can advance through the internal frames
	template<IsBuffer Buffer, size_t Frames = 3zu>
	class MultiBuffer
	{
	public:
		MultiBuffer(size_t size, std::string_view name)
			: m_Buffer{ size * Frames, name }
			, m_Size{ size }
			, m_FrameOffset{}
		{}

		void Write(DataBufferView data, size_t offset)
		{
			m_Buffer.Write(data, offset + m_FrameOffset);
		}

		void Advance()
		{
			m_FrameOffset = (m_FrameOffset + m_Size) % (m_Size * Frames);
		}

		auto GetNativeHandle() const
		{
			return m_Buffer.GetNativeHandle();
		}

		const Buffer& GetBuffer() const
		{
			return m_Buffer;
		}

		size_t GetSize() const
		{
			return m_Size;
		}

		size_t FrameOffsetBytes() const
		{
			return m_FrameOffset;
		}

		std::string_view GetName() const
		{
			return m_Buffer.GetName();
		}

	private:
		Buffer m_Buffer;
		size_t m_Size;
		size_t m_FrameOffset;
	};

}
