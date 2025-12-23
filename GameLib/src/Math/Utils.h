#pragma once

#include "Ray.h"
#include "Vector3.h"

#include <optional>

namespace Game
{

	inline std::optional<float> Intersect(const Ray& ray, const vec3& v0, const vec3& v1, const vec3& v2)
	{
		const auto edge1 = v1 - v0;
		const auto edge2 = v2 - v0;

		const auto h = vec3::Cross(ray.direction, edge2);
		const auto a = vec3::Dot(edge1, h);

		if (std::abs(a) < 1e-8f)
		{
			return {};
		}

		const auto f = 1.0f / a;
		const auto s = ray.origin - v0;
		const auto u = f * vec3::Dot(s, h);

		if (u < 0.0f || u > 1.0f)
		{
			return {};
		}

		const auto q = vec3::Cross(s, edge1);
		const auto v = f * vec3::Dot(ray.direction, q);

		if (v < 0.0f || u + v > 1.0f)
		{
			return {};
		}

		const auto t = f * vec3::Dot(edge2, q);
		return t > 1e-8f ? std::make_optional(t) : std::nullopt;
	}

}
