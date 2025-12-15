#pragma once

#include "Math/Matrix4.h"

#include <cstdint>

namespace Game {

	struct ObjectData
	{
		mat4 model;
		uint32_t materialIDIndex;
		uint32_t padding[3];
	};

}
