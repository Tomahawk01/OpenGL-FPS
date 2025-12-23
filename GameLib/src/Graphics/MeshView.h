#pragma once

#include "Graphics/VertexData.h"

#include <cstdint>
#include <span>

namespace Game {

	struct MeshView
	{
		uint32_t indexOffset;
		uint32_t indexCount;
		uint32_t vertexOffset;
		uint32_t vertexCount;
	};

}