#pragma once

#include <format>

namespace {

	constexpr float custom_sqrt(float x) noexcept
	{
		if (x < 0) return NAN;

		float guess = x;
		float prev = 0.0f;

		while (guess != prev)
		{
			prev = guess;
			guess = 0.5f * (guess + x / guess);
		}
		return guess;
	}

	constexpr float custom_hypot(float x, float y, float z)
	{
		return custom_sqrt(x * x + y * y + z * z);
	}

}

namespace Game {

	struct vec3
	{
		constexpr vec3()
			: vec3(0.0f)
		{}

		constexpr vec3(float xyz)
			: vec3(xyz, xyz, xyz)
		{}

		constexpr vec3(float x, float y, float z)
			: x(x), y(y), z(z)
		{}

		constexpr float Length() const
		{
			return custom_hypot(x, y, z);
		}

		static constexpr vec3 Normalize(const vec3& v)
		{
			const float l = v.Length();
			if (l == 0.0f)
				return {};

			return { v.x / l, v.y / l, v.z / l };
		}

		static constexpr vec3 Cross(const vec3& v1, const vec3& v2)
		{
			const float i = (v1.y * v2.z) - (v1.z * v2.y);
			const float j = (v1.x * v2.z) - (v1.z * v2.x);
			const float k = (v1.x * v2.y) - (v1.y * v2.x);

			return { i, -j, k };
		}

		static constexpr float Dot(const vec3& v1, const vec3& v2)
		{
			return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
		}

		static constexpr float Distance(const vec3& v1, const vec3& v2);

		constexpr bool operator==(const vec3&) const = default;

		std::string to_string() const;

		float x;
		float y;
		float z;
	};

	constexpr vec3& operator+=(vec3& v1, const vec3& v2)
	{
		v1.x += v2.x;
		v1.y += v2.y;
		v1.z += v2.z;

		return v1;
	}

	constexpr vec3 operator+(const vec3& v1, const vec3& v2)
	{
		vec3 temp = v1;
		return temp += v2;
	}

	constexpr vec3& operator-=(vec3& v1, const vec3& v2)
	{
		v1.x -= v2.x;
		v1.y -= v2.y;
		v1.z -= v2.z;

		return v1;
	}

	constexpr vec3 operator-(const vec3& v1, const vec3& v2)
	{
		vec3 temp = v1;
		return temp -= v2;
	}

	constexpr vec3 operator-(const vec3& v)
	{
		return { -v.x, -v.y, -v.z };
	}

	constexpr vec3& operator*=(vec3& v1, const vec3& v2)
	{
		v1.x *= v2.x;
		v1.y *= v2.y;
		v1.z *= v2.z;

		return v1;
	}

	constexpr vec3 operator*(const vec3& v1, const vec3& v2)
	{
		vec3 temp = v1;
		return temp *= v2;
	}

	constexpr vec3& operator/=(vec3& v1, const vec3& v2)
	{
		v1.x /= v2.x;
		v1.y /= v2.y;
		v1.z /= v2.z;

		return v1;
	}

	constexpr vec3 operator/(const vec3& v1, const vec3& v2)
	{
		vec3 temp = v1;
		return temp /= v2;
	}

	constexpr float vec3::Distance(const vec3& v1, const vec3& v2)
	{
		return (v2 - v1).Length();
	}

	std::string vec3::to_string() const
	{
		return std::format("x={} y={} z={}", x, y, z);
	}

}
