#pragma once

#include "Math/Vector3.h"
#include "Color.h"

namespace Game {

	struct PointLight
	{
		vec3 position;
		Color color;
		float constantAttenuation;
		float linearAttenuation;
		float quadraticAttenuation;
	};

	static_assert(sizeof(PointLight) == sizeof(float) * 9);

}
