#pragma once

#include "Math/Matrix4.h"
#include "Math/Vector3.h"
#include "Utils/DataBuffer.h"

#include <array>
#include <span>

namespace Game {

	struct CameraData
	{
		mat4 view;
		mat4 projection;
		vec3 position;
		float pad = 0.0f;
	};

	class Camera
	{
	public:
		Camera(const vec3& position,
			   const vec3& lookAt,
			   const vec3& up,
			   float fov,
			   float width, float height,
			   float nearPlane, float farPlane);

		Camera(float width, float height, float depth);

		void Translate(const vec3& translation);

		void AddYaw(float value);
		void SetYaw(float yaw);
		void AddPitch(float value);
		void SetPitch(float pitch);

		vec3 GetPosition() const;
		void SetPosition(const vec3& position);
		vec3 GetDirection() const;
		vec3 Up() const;
		vec3 Right() const;
		float GetFOV() const;
		float GetWidth() const;
		float GetHeight() const;
		float GetNearPlane() const;
		float GetFarPlane() const;
		const CameraData& GetData() const;
		DataBufferView GetDataView() const;

	private:
		CameraData m_Data;

		vec3 m_Direction;
		vec3 m_Up;
		vec3 m_Right;

		float m_Pitch;
		float m_Yaw;

		float m_Fov;
		float m_Width;
		float m_Height;
		float m_NearPlane;
		float m_FarPlane;
	};

}
