#pragma once

#include "Core/Camera.h"
#include "Entity.h"
#include "MeshManager.h"
#include "MaterialManager.h"

#include <vector>

namespace Game {

	struct Scene
	{
		std::vector<Entity> entities;
		MeshManager& meshManager;
		MaterialManager& materialManager;
		Camera camera;
	};

}
