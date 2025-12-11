#include "MeshManager.h"

#include "Utils/Log.h"

#include <ranges>

namespace {

	template<class T>
	void ResizeGPUBuffer(const std::vector<T>& cpuBuffer, Game::Buffer& gpuBuffer, std::string_view name)
	{
		const auto bufferSizeBytes = cpuBuffer.size() * sizeof(T);
		if (gpuBuffer.GetSize() <= bufferSizeBytes)
		{
			auto newSize = gpuBuffer.GetSize() * 2;
			while (newSize < bufferSizeBytes)
			{
				newSize *= 2;
			}

			Game::Log::Info("Growing {} buffer {} -> {}", name, gpuBuffer.GetSize(), newSize);

			// OpenGL barrier in case gpu using previous frame
			glFinish();

			gpuBuffer = Game::Buffer{ newSize, name };
		}
	}

}

namespace Game {

	MeshManager::MeshManager()
		: m_VertexDataCPU{}
		, m_IndexDataCPU{}
		, m_VertexDataGPU{ sizeof(VertexData), "vertex_mesh_data" }
		, m_IndexDataGPU{ sizeof(uint32_t), "index_mesh_data" }
	{}

	MeshView MeshManager::Load(const MeshData& meshData)
	{
		const auto vertexOffset = m_VertexDataCPU.size();
		const auto indexOffset = m_IndexDataCPU.size();

		m_VertexDataCPU.append_range(meshData.vertices);
		ResizeGPUBuffer(m_VertexDataCPU, m_VertexDataGPU, "vertex_mesh_data");
		const auto vertexDataView = DataBufferView{ reinterpret_cast<const std::byte*>(m_VertexDataCPU.data()), m_VertexDataCPU.size() * sizeof(VertexData) };
		m_VertexDataGPU.Write(vertexDataView, 0u);

		m_IndexDataCPU.append_range(meshData.indices);
		ResizeGPUBuffer(m_IndexDataCPU, m_IndexDataGPU, "index_mesh_data");
		const auto indexDataView = DataBufferView{ reinterpret_cast<const std::byte*>(m_IndexDataCPU.data()), m_IndexDataCPU.size() * sizeof(uint32_t) };
		m_IndexDataGPU.Write(indexDataView, 0u);

		return {
			.indexOffset = static_cast<uint32_t>(indexOffset),
			.indexCount = static_cast<uint32_t>(meshData.indices.size()),
			.vertexOffset = static_cast<uint32_t>(vertexOffset),
			.vertexCount = static_cast<uint32_t>(meshData.vertices.size())
		};
	}

	std::tuple<GLuint, GLuint> MeshManager::GetNativeHandle() const
	{
		return { m_VertexDataGPU.GetNativeHandle(), m_IndexDataGPU.GetNativeHandle() };
	}

	std::string MeshManager::to_string() const
	{
		return std::format("Mesh manager: vertex count {}, index count {}", m_VertexDataCPU.size(), m_IndexDataCPU.size());
	}

}
