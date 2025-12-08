#include "CommandBuffer.h"

#include "Utils/Log.h"

#include <cstdint>
#include <ranges>
#include <format>

namespace {

	struct IndirectCommand
	{
		uint32_t count;
		uint32_t instanceCount;
		uint32_t first;
		uint32_t baseInstance;
	};

}

namespace Game {

	CommandBuffer::CommandBuffer()
		: m_CommandBuffer{ 1u, "command_buffer" }
	{}

	uint32_t CommandBuffer::Build(const Scene& scene)
	{
		const auto command = scene.entities | std::views::transform(
			[](const auto& e)
			{
				return IndirectCommand{
					.count = e.meshView.count,
					.instanceCount = 1u,
					.first = e.meshView.offset,
					.baseInstance = 0u };
			}) | std::ranges::to<std::vector>();

		const auto commandView = DataBufferView{ reinterpret_cast<const std::byte*>(command.data()), command.size() * sizeof(IndirectCommand) };

		if (commandView.size_bytes() > m_CommandBuffer.GetOriginalSize())
		{
			auto newSize = m_CommandBuffer.GetOriginalSize() * 2;
			while (newSize < commandView.size_bytes())
			{
				newSize *= 2;
			}

			Log::Info("Growing command buffer {} -> {}", m_CommandBuffer.GetOriginalSize(), newSize);

			// OpenGL barrier in case gpu using previous frame
			glFinish();

			m_CommandBuffer = MultiBuffer<PersistentBuffer>{ newSize, "mesh_data" };
		}

		m_CommandBuffer.Write(commandView, 0u);

		return command.size();
	}

	void CommandBuffer::Advance()
	{
		m_CommandBuffer.Advance();
	}

	GLuint CommandBuffer::GetNativeHandle() const
	{
		return m_CommandBuffer.GetBuffer().GetNativeHandle();
	}

	std::string CommandBuffer::to_string() const
	{
		return std::format("Command buffer {} size", m_CommandBuffer.GetOriginalSize());
	}

}
