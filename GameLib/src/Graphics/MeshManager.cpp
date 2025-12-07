#include "MeshManager.h"

#include "Utils/Log.h"

#include <ranges>

namespace Game {

	MeshManager::MeshManager()
		: m_MeshDataCPU{}
		, m_MeshDataGPU{ sizeof(VertexData), "mesh_data" }
	{
		m_MeshDataCPU.reserve(1u);
	}

	MeshView MeshManager::Load(const std::vector<VertexData>& mesh)
	{
		const auto offset = m_MeshDataCPU.size();
		m_MeshDataCPU.append_range(mesh);

		const auto bufferSizeBytes = m_MeshDataCPU.size() * sizeof(VertexData);
		if (m_MeshDataGPU.GetSize() <= bufferSizeBytes)
		{
			auto newSize = m_MeshDataGPU.GetSize() * 2;
			while (newSize < bufferSizeBytes)
			{
				newSize *= 2;
			}

			Log::Info("Growing mesh_data buffer {:x} -> {:x}", m_MeshDataGPU.GetSize(), newSize);
			m_MeshDataGPU = Buffer{ newSize, "mesh_data" };
		}

		auto meshView = DataBufferView{ reinterpret_cast<const std::byte*>(m_MeshDataCPU.data()), bufferSizeBytes };
		m_MeshDataGPU.Write(meshView, 0u);

		return {
			.offset = static_cast<uint32_t>(offset),
			.count = static_cast<uint32_t>(mesh.size())
		};
	}

	GLuint MeshManager::GetNativeHandle() const
	{
		return m_MeshDataGPU.GetNativeHandle();
	}

	std::string MeshManager::to_string() const
	{
		return std::format("Mesh manager: vertex count: {:x}", m_MeshDataCPU.size());
	}

}
