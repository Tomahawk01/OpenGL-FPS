#pragma once

#include "Core/Camera.h"
#include "Core/Entity.h"
#include "Graphics/MeshManager.h"
#include "Graphics/MaterialManager.h"
#include "Graphics/Texture.h"
#include "Math/Utils.h"
#include "Math/Ray.h"
#include "Math/Vector4.h"

#include "Graphics/PointLight.h"

#include <optional>
#include <ranges>
#include <vector>

namespace Game {

	struct IntersectionResult
	{
		const Entity* entity;
		vec3 position;
	};

	struct LightData
	{
		Color ambient;
		PointLight light;
	};

	struct Scene
	{
		constexpr std::optional<IntersectionResult> IntersectRay(const Ray& ray) const
		{
			auto result = std::optional<IntersectionResult>{};
			auto minDistance = std::numeric_limits<float>::max();

			for (const auto& entity : entities)
			{
				const auto invTransform = mat4::Invert(entity.transform);
				const auto transformedRay = Ray{ invTransform * vec4{ray.origin, 1.0f}, invTransform * vec4{ray.direction, 0.0f} };

				const auto indices = meshManager.GetIndexData(entity.meshView);
				const auto vertices = meshManager.GetVertexData(entity.meshView);

				for (const auto& indices : std::views::chunk(indices, 3))
				{
					const auto v0 = vertices[indices[0]].position;
					const auto v1 = vertices[indices[1]].position;
					const auto v2 = vertices[indices[2]].position;

					if (const auto distance = Intersect(transformedRay, v0, v1, v2); distance)
					{
						const auto intersectionPoint = transformedRay.origin + transformedRay.direction * (*distance);

						if (*distance < minDistance)
						{
							result = IntersectionResult{ .entity = &entity, .position = intersectionPoint };
							minDistance = *distance;
						}
					}
				}
			}

			return result;
		}

		std::vector<Entity> entities;
		MeshManager& meshManager;
		MaterialManager& materialManager;
		Camera camera;
		const Texture& theOneTexture;
		LightData lights;
	};

}
