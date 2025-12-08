#pragma once

#include "Entity.h"
#include "MeshManager.h"

#include <vector>

namespace Game {

	struct Scene
	{
		std::vector<Entity> entities;
		MeshManager& meshManager;
	};

}
