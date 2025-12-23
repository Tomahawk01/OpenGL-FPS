#pragma once

#include "Core/Camera.h"
#include "Core/Entity.h"
#include "Graphics/MeshManager.h"
#include "Graphics/MaterialManager.h"
#include "Math/Utils.h"
#include "Math/Ray.h"
#include "Math/Vector4.h"

#include <optional>
#include <ranges>
#include <vector>

namespace Game {

	struct IntersctionResult
	{
		const Entity* entity;
		vec3 position;
	};

	struct Scene
	{
		constexpr std::optional<IntersctionResult> IntersectRay(const Ray& ray) const
		{
			auto result = std::optional<IntersctionResult>{};
			auto min_distance = std::numeric_limits<float>::max();

			for (const auto& entity : entities)
			{
				const auto inv_transform = mat4::Invert(entity.transform);
				const auto transformed_ray = Ray{ inv_transform * vec4{ray.origin, 1.0f}, inv_transform * vec4{ray.direction, 0.0f} };

				for (const auto& indices : std::views::chunk(entity.meshView.indices, 3))
				{
					const auto v0 = entity.meshView.vertices[indices[0]].position;
					const auto v1 = entity.meshView.vertices[indices[1]].position;
					const auto v2 = entity.meshView.vertices[indices[2]].position;

					if (const auto distance = Intersect(transformed_ray, v0, v1, v2); distance)
					{
						const auto intersection_point = transformed_ray.origin + transformed_ray.direction * (*distance);

						if (!result && (*distance < min_distance))
						{
							result = IntersctionResult{ .entity = &entity, .position = intersection_point };
							min_distance = *distance;
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
	};

}
