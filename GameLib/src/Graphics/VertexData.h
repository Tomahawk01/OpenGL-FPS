#pragma once

#include "Math/Vector3.h"
#include "Color.h"

namespace Game {

	struct UV
	{
		float s;
		float t;
	};

	struct VertexData
	{
		vec3 position;
		vec3 normal;
		vec3 tangent;
		vec3 bitangent;
		UV uv;
	};

	static_assert(sizeof(VertexData) == sizeof(float) * 3 + sizeof(float) * 3 + sizeof(float) * 3 + sizeof(float) * 3 + sizeof(float) * 2);

}
