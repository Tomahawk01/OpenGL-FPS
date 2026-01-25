#pragma once

#include "Buffer.h"
#include "Utils.h"
#include "Utils/Error.h"

#include <cstdint>
#include <span>
#include <ranges>

namespace Game {

	struct MaterialData
	{
		uint32_t albedoTextureIndex;
		uint32_t normalTextureIndex;
		uint32_t specularTextureIndex;
	};

	class MaterialManager
	{
	public:
		MaterialManager()
			: m_MaterialDataCPU{}
			, m_MaterialDataGPU{ sizeof(MaterialData), "material_manager_buffer" }
		{}

		template<class... Args>
		uint32_t Add(Args &&...args)
		{
			const auto newIndex = m_MaterialDataCPU.size();

			m_MaterialDataCPU.emplace_back(std::forward<Args>(args)...);

			ResizeGPUBuffer(m_MaterialDataCPU, m_MaterialDataGPU);

			m_MaterialDataGPU.Write(std::as_bytes(std::span{ m_MaterialDataCPU.data(), m_MaterialDataCPU.size() }), 0zu);

			return newIndex;
		}

		const std::vector<MaterialData>& Data() const
		{
			return m_MaterialDataCPU;
		}

		auto GetNativeHandle() const
		{
			return m_MaterialDataGPU.GetNativeHandle();
		}

	private:
		std::vector<MaterialData> m_MaterialDataCPU;
		Buffer m_MaterialDataGPU;
	};

}
