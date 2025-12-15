#pragma once

#include "Graphics/MeshView.h"
#include "Graphics/MaterialManager.h"
#include "Math/Transform.h"

namespace Game {

	struct Entity
	{
		MeshView meshView;
		Transform transform;
		MaterialKey materialKey;
	};

}
