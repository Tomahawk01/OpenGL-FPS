#pragma once

#include "Buffer.h"
#include "MeshView.h"
#include "VertexData.h"
#include "MeshData.h"

#include <vector>
#include <span>
#include <string>

namespace Game {

	class MeshManager
	{
	public:
		MeshManager();

		MeshView Load(const MeshData& meshData);

		std::tuple<GLuint, GLuint> GetNativeHandle() const;

		std::span<uint32_t> GetIndexData(MeshView view);
		std::span<VertexData> GetVertexData(MeshView view);

		std::string to_string() const;

	private:
		std::vector<VertexData> m_VertexDataCPU;
		std::vector<uint32_t> m_IndexDataCPU;
		Buffer m_VertexDataGPU;
		Buffer m_IndexDataGPU;
	};

}
