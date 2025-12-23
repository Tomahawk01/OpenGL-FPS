#pragma once

#include "Vector3.h"

#include <cmath>
#include <format>

namespace Game {

	class vec4
	{
	public:
		constexpr vec4()
			: vec4(0.0f)
		{}

		constexpr vec4(float xyzw)
			: vec4(xyzw, xyzw, xyzw, xyzw)
		{}

		constexpr vec4(const vec3& v3, float w)
			: vec4(v3.x, v3.y, v3.z, w)
		{}

		constexpr vec4(float x, float y, float z, float w)
			: x(x), y(y), z(z), w(w)
		{}

		constexpr operator vec3() const;

		constexpr bool operator==(const vec4&) const = default;

		inline std::string to_string() const;

		float x;
		float y;
		float z;
		float w;
	};

	constexpr vec4::operator vec3() const
	{
		return vec3{ x, y, z };
	}

	inline std::string vec4::to_string() const
	{
		return std::format("x={} y={} z={} w={}", x, y, z, w);
	}

}
