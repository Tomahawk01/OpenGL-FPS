#include "CommandBuffer.h"

#include "Utils.h"
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
		int32_t baseVertex;
		uint32_t baseInstance;
	};

}

namespace Game {

	CommandBuffer::CommandBuffer()
		: m_CommandBuffer{ 1u, "command_buffer" }
	{}

	uint32_t CommandBuffer::Build(const Scene& scene)
	{
		auto base = 0;
		const auto command = scene.entities | std::views::transform(
			[&base](const auto& e)
			{
				const auto cmd = IndirectCommand{
					.count = static_cast<uint32_t>(std::ranges::size(e.meshView.indices)),
					.instanceCount = 1u,
					.first = e.meshView.indexOffset,
					.baseVertex = base,
					.baseInstance = 0u };
				base += e.meshView.vertexOffset;
				return cmd;
			}) | std::ranges::to<std::vector>();

		const auto commandView = DataBufferView{ reinterpret_cast<const std::byte*>(command.data()), command.size() * sizeof(IndirectCommand) };

		ResizeGPUBuffer(command, m_CommandBuffer, "command_buffer");

		m_CommandBuffer.Write(commandView, 0u);

		return command.size();
	}

	void CommandBuffer::Advance()
	{
		m_CommandBuffer.Advance();
	}

	size_t CommandBuffer::OffsetBytes() const
	{
		return m_CommandBuffer.FrameOffsetBytes();
	}

	GLuint CommandBuffer::GetNativeHandle() const
	{
		return m_CommandBuffer.GetBuffer().GetNativeHandle();
	}

	std::string CommandBuffer::to_string() const
	{
		return std::format("Command buffer {} size", m_CommandBuffer.GetSize());
	}

}
