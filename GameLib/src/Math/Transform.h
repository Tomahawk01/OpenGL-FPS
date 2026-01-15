#pragma once

#include "Math/Vector3.h"
#include "Math/Matrix4.h"
#include "Math/Quaternion.h"
#include "Utils/Formatter.h"

#include <format>

namespace Game {

	class Transform
	{
	public:
		constexpr Transform()
			: Position{}
			, Scale{ 1.0f }
			, Rotation{}
		{}

		constexpr Transform(const vec3& position, const vec3& scale, const quat& rotation)
			: Position{ position }
			, Scale{ scale }
			, Rotation{ rotation }
		{}

		constexpr Transform(const mat4& transform)
			: Position{}
			, Scale{}
			, Rotation{}
		{
			Position = vec3{ transform[12], transform[13], transform[14] };
			Scale = vec3{
				vec3{transform[0], transform[1], transform[2]}.Length(),
				vec3{transform[4], transform[5], transform[6]}.Length(),
				vec3{transform[8], transform[9], transform[10]}.Length()
			};

			auto trace = 0.0f;
			auto s = 0.0f;

			const auto scaleX = std::sqrt(transform[0] * transform[0] + transform[1] * transform[1] + transform[2] * transform[2]);
			const auto scaleY = std::sqrt(transform[4] * transform[4] + transform[5] * transform[5] + transform[6] * transform[6]);
			const auto scaleZ = std::sqrt(transform[8] * transform[8] + transform[9] * transform[9] + transform[10] * transform[10]);

			const auto m00 = transform[0] / scaleX;
			const auto m10 = transform[1] / scaleX;
			const auto m20 = transform[2] / scaleX;

			const auto m01 = transform[4] / scaleY;
			const auto m11 = transform[5] / scaleY;
			const auto m21 = transform[6] / scaleY;

			const auto m02 = transform[8] / scaleZ;
			const auto m12 = transform[9] / scaleZ;
			const auto m22 = transform[10] / scaleZ;

			trace = m00 + m11 + m22;

			if (trace > 0.0f)
			{
				s = 0.5f / std::sqrt(trace + 1.0f);
				Rotation.W = 0.25f / s;
				Rotation.X = (m21 - m12) * s;
				Rotation.Y = (m02 - m20) * s;
				Rotation.Z = (m10 - m01) * s;
			}
			else
			{
				if (m00 > m11 && m00 > m22)
				{
					s = 2.0f * std::sqrt(1.0f + m00 - m11 - m22);
					Rotation.W = (m21 - m12) / s;
					Rotation.X = 0.25f * s;
					Rotation.Y = (m01 + m10) / s;
					Rotation.Z = (m02 + m20) / s;
				}
				else if (m11 > m22)
				{
					s = 2.0f * std::sqrt(1.0f + m11 - m00 - m22);
					Rotation.W = (m02 - m20) / s;
					Rotation.X = (m01 + m10) / s;
					Rotation.Y = 0.25f * s;
					Rotation.Z = (m12 + m21) / s;
				}
				else
				{
					s = 2.0f * std::sqrt(1.0f + m22 - m00 - m11);
					Rotation.W = (m10 - m01) / s;
					Rotation.X = (m02 + m20) / s;
					Rotation.Y = (m12 + m21) / s;
					Rotation.Z = 0.25f * s;
				}
			}
		}

		constexpr operator mat4() const
		{
			return mat4{ Position } * mat4{ Rotation } * mat4{ Scale, mat4::Scale{} };
		}

		vec3 Position;
		vec3 Scale;
		quat Rotation;
	};

	inline std::string to_string(const Transform& transform)
	{
		return std::format("pos:{}, scale:{}, rot:{}", transform.Position, transform.Scale, transform.Rotation);
	}

}
