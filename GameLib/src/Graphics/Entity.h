#pragma once

#include "MeshView.h"
#include "Math/Transform.h"

namespace Game {

	struct Entity
	{
		MeshView meshView;
		Transform transform;
	};

}
