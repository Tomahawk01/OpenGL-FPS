#pragma once

#include "Graphics/VertexData.h"

#include <cstdint>
#include <span>

namespace Game {

	struct MeshView
	{
		uint32_t indexOffset;
		std::span<uint32_t> indices;
		uint32_t vertexOffset;
		std::span<VertexData> vertices;
	};

}