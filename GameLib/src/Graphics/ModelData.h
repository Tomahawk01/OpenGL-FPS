#pragma once

#include "MeshData.h"
#include "Texture.h"

#include <optional>

namespace Game {

	struct ModelData
	{
		MeshData meshData;
		std::optional<Texture> albedo;
		std::optional<Texture> normal;
		std::optional<Texture> specular;
	};

}
