#pragma once

#include "VertexData.h"

#include <cstdint>
#include <format>
#include <string>
#include <vector>

namespace Game {

	struct MeshData
	{
		std::vector<VertexData> vertices;
		std::vector<uint32_t> indices;
	};

	inline std::string to_string(const MeshData& data)
	{
		return std::format("Mesh data: v {}, i {}", data.vertices.size(), data.indices.size());
	}

}
