#pragma once

#include "MeshData.h"
#include "TextureData.h"

#include <optional>

namespace Game {

	struct ModelData
	{
		MeshData meshData;
		std::optional<TextureData> albedo;
		std::optional<TextureData> normal;
		std::optional<TextureData> specular;
	};

}
