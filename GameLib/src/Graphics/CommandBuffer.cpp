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

	CommandBuffer::CommandBuffer(std::string_view name)
		: m_CommandBuffer{ 1u, name }
	{}

	uint32_t CommandBuffer::Build(const Scene& scene)
	{
		const auto command = scene.entities | std::views::transform(
			[](const auto& e)
			{
				const auto cmd = IndirectCommand{
					.count = e.meshView.indexCount,
					.instanceCount = 1u,
					.first = e.meshView.indexOffset,
					.baseVertex = static_cast<int32_t>(e.meshView.vertexOffset),
					.baseInstance = 0u
				};
				return cmd;
			}) | std::ranges::to<std::vector>();

		const auto commandView = DataBufferView{ reinterpret_cast<const std::byte*>(command.data()), command.size() * sizeof(IndirectCommand) };

		ResizeGPUBuffer(command, m_CommandBuffer, "command_buffer");

		m_CommandBuffer.Write(commandView, 0u);

		return command.size();
	}

	uint32_t CommandBuffer::Build(const Entity& entity)
	{
		const auto cmd = IndirectCommand{
			.count = entity.meshView.indexCount,
			.instanceCount = 1u,
			.first = entity.meshView.indexOffset,
			.baseVertex = 0u,
			.baseInstance = 0u
		};
		const auto commandView = std::as_bytes(std::span{&cmd, 1});

		ResizeGPUBuffer(std::vector<IndirectCommand>{ cmd }, m_CommandBuffer, "command_buffer");

		m_CommandBuffer.Write(commandView, 0u);

		return 1u;
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

	std::string_view CommandBuffer::GetName() const
	{
		return m_CommandBuffer.GetName();
	}

	std::string CommandBuffer::to_string() const
	{
		return std::format("Command buffer {} size", m_CommandBuffer.GetSize());
	}

}
