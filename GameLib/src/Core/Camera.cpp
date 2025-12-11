#include "Camera.h"

#include <numbers>

namespace {

	constexpr Game::vec3 CreateDirection(float pitch, float yaw)
	{
		return Game::vec3::Normalize({
			std::cos(pitch) * std::cos(yaw),
			std::sin(pitch),
			std::cos(pitch) * std::sin(yaw)
		});
	}

}

namespace Game {

	Camera::Camera(const vec3& position,
				   const vec3& lookAt,
				   const vec3& up,
				   float fov,
				   float width, float height,
				   float nearPlane, float farPlane)
		: m_Data{
			.view = mat4::LookAt(position, lookAt, up),
			.projection = mat4::Perspective(fov, width, height, nearPlane, farPlane)
		}
		, m_Position(position)
		, m_Direction(lookAt)
		, m_Up(up)
		, m_Right(vec3::Normalize(vec3::Cross(m_Direction, m_Up)))
		, m_Pitch{}
		, m_Yaw{ -std::numbers::pi_v<float> / 2.0f }
		, m_Fov(fov)
		, m_Width(width)
		, m_Height(height)
		, m_NearPlane(nearPlane)
		, m_FarPlane(farPlane)
	{
		m_Direction = CreateDirection(m_Pitch, m_Yaw);
		m_Data.view = mat4::LookAt(m_Position, m_Position + m_Direction, m_Up);
		AddPitch(0.0f);
	}

	Camera::Camera(float width, float height, float depth)
		: m_Data{
			.view = mat4::LookAt(vec3{ 0.0f, 0.0f, 1.0f }, {}, { 0.0f, 1.0f, 0.0f }),
			.projection = mat4::Orthographic(width, height, depth)
		}
		, m_Position(vec3{ 0.0f, 0.0f, 1.0f })
		, m_Direction(vec3{ 0.0f, 0.0f, -1.0f })
		, m_Up(vec3{ 0.0f, 1.0f, 0.0f })
		, m_Right(vec3::Normalize(vec3::Cross(m_Direction, m_Up)))
		, m_Pitch{}
		, m_Yaw{ -std::numbers::pi_v<float> / 2.0f }
		, m_Fov(0.0f)
		, m_Width(width)
		, m_Height(height)
		, m_NearPlane(0.0f)
		, m_FarPlane(depth)
	{}

	void Camera::Translate(const vec3& translation)
	{
		m_Position += translation;
		m_Direction = CreateDirection(m_Pitch, m_Yaw);
		m_Data.view = mat4::LookAt(m_Position, m_Position + m_Direction, m_Up);
	}

	void Camera::AddYaw(float value)
	{
		m_Yaw += value;
		m_Direction = CreateDirection(m_Pitch, m_Yaw);

		const vec3 worldUp{ 0.0f, 1.0f, 0.0f };
		m_Right = vec3::Normalize(vec3::Cross(m_Direction, worldUp));
		m_Up = vec3::Normalize(vec3::Cross(m_Right, m_Direction));

		m_Data.view = mat4::LookAt(m_Position, m_Position + m_Direction, m_Up);
	}

	void Camera::SetYaw(float yaw)
	{
		AddYaw(yaw - m_Yaw);
	}

	void Camera::AddPitch(float value)
	{
		m_Pitch += value;
		constexpr float pitchEpsilon = 0.0001f;
		m_Pitch = std::clamp(m_Pitch, (-std::numbers::pi_v<float> / 2.0f) + pitchEpsilon, (std::numbers::pi_v<float> / 2.0f) - pitchEpsilon);
		m_Direction = CreateDirection(m_Pitch, m_Yaw);

		const vec3 worldUp{ 0.0f, 1.0f, 0.0f };
		m_Right = vec3::Normalize(vec3::Cross(m_Direction, worldUp));
		m_Up = vec3::Normalize(vec3::Cross(m_Right, m_Direction));

		m_Data.view = mat4::LookAt(m_Position, m_Position + m_Direction, m_Up);
	}

	void Camera::SetPitch(float pitch)
	{
		AddPitch(pitch - m_Pitch);
	}

	vec3 Camera::GetPosition() const
	{
		return m_Position;
	}

	void Camera::SetPosition(const vec3& position)
	{
		m_Position = position;
		m_Data.view = mat4::LookAt(m_Position, m_Position + m_Direction, m_Up);
	}

	vec3 Camera::GetDirection() const
	{
		return m_Direction;
	}

	vec3 Camera::Up() const
	{
		return m_Up;
	}

	vec3 Camera::Right() const
	{
		return m_Right;
	}

	float Camera::GetFOV() const
	{
		return m_Fov;
	}

	float Camera::GetWidth() const
	{
		return m_Width;
	}

	float Camera::GetHeight() const
	{
		return m_Height;
	}

	float Camera::GetNearPlane() const
	{
		return m_NearPlane;
	}

	float Camera::GetFarPlane() const
	{
		return m_FarPlane;
	}

	const CameraData& Camera::GetData() const
	{
		return m_Data;
	}

	DataBufferView Camera::GetDataView() const
	{
		return { reinterpret_cast<const std::byte*>(&m_Data), sizeof(m_Data) };
	}

}
