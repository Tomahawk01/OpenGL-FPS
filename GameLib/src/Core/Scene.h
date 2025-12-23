#pragma once

#include "Core/Camera.h"
#include "Core/Entity.h"
#include "Graphics/MeshManager.h"
#include "Graphics/MaterialManager.h"

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
