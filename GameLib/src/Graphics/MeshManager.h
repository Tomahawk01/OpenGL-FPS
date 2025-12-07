#pragma once

#include "VertexData.h"
#include "Buffer.h"

#include <cstdint>
#include <vector>
#include <string>

namespace Game {

	struct MeshView
	{
		uint32_t offset;
		uint32_t count;
	};

	class MeshManager
	{
	public:
		MeshManager();

		MeshView Load(const std::vector<VertexData>& mesh);

		GLuint GetNativeHandle() const;
		std::string to_string() const;

	private:
		std::vector<VertexData> m_MeshDataCPU;
		Buffer m_MeshDataGPU;
	};

}
