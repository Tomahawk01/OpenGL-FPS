#pragma once

#include "Graphics/MeshView.h"
#include "Math/Transform.h"

#include <cstdint>
#include <string>

namespace Game {

	struct Entity
	{
		std::string name;
		MeshView meshView;
		Transform transform;
		uint32_t materialIndex;
	};

}
