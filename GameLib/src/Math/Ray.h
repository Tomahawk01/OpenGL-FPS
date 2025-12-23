#pragma once

#include "Vector3.h"

namespace Game
{

	class Ray
	{
	public:
		constexpr Ray(const vec3& origin, const vec3& direction)
			: origin{ origin }
			, direction{ vec3::Normalize(direction) }
		{}

		vec3 origin;
		vec3 direction;
	};

}
