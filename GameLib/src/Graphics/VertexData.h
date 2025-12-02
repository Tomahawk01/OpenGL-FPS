#pragma once

#include "Math/Vector3.h"
#include "Color.h"

namespace Game {

	struct VertexData
	{
		vec3 position;
		Color color;
	};

	static_assert(sizeof(VertexData) == sizeof(float) * 3 + sizeof(float) * 3);

}
