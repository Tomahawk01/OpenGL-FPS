#pragma once

#include "Graphics/MeshView.h"
#include "Graphics/MaterialManager.h"
#include "Math/Transform.h"

#include <string>

namespace Game {

	struct Entity
	{
		std::string name;
		MeshView meshView;
		Transform transform;
		MaterialKey materialKey;
	};

}
