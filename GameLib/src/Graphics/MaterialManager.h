#pragma once

#include "Color.h"
#include "PersistentBuffer.h"
#include "MultiBuffer.h"
#include "Utils.h"
#include "Utils/Error.h"
#include "Utils/Formatter.h"

#include <cstdint>
#include <string>
#include <ranges>
#include <compare>

#include <flat_map.h>

namespace Game {

	class MaterialKey
	{
	public:
		constexpr MaterialKey(uint32_t key)
			: m_Key{ key }
		{}

		constexpr uint32_t Get() const
		{
			return m_Key;
		}

		constexpr uint32_t operator*() const
		{
			return Get();
		}

		constexpr auto operator<=>(const MaterialKey&) const = default;

		std::string to_string() const
		{
			return std::to_string(m_Key);
		}

	private:
		uint32_t m_Key;
	};

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
		MaterialKey Add(Args&&... args)
		{
			static auto keyNum = 0u;
			const auto key = MaterialKey{ keyNum++ };

			m_MaterialDataCPU.emplace(key, MaterialData{ std::forward<Args>(args)... });
			ResizeGPUBuffer(Data(), m_MaterialDataGPU, "material_manager_buffer");

			return key;
		}

		MaterialData& operator[](MaterialKey key)
		{
			const auto element = m_MaterialDataCPU.find(key);
			Expect(element != std::ranges::cend(m_MaterialDataCPU), "Key {} does not exist", key);

			return element->second;
		}

		void Remove(MaterialKey key)
		{
			m_MaterialDataCPU.erase(key);
		}

		uint32_t Index(MaterialKey key)
		{
			const auto element = m_MaterialDataCPU.find(key);
			Expect(element != std::ranges::cend(m_MaterialDataCPU), "Could not find key: {}", key);

			return static_cast<uint32_t>(std::ranges::distance(std::ranges::cbegin(m_MaterialDataCPU), element));
		}

		const std::vector<MaterialData>& Data() const
		{
			return m_MaterialDataCPU.values();
		}

		void Sync()
		{
			const auto& values = m_MaterialDataCPU.values();
			m_MaterialDataGPU.Write(std::as_bytes(std::span{ values.data(), values.size() }), 0);
		}

		auto GetNativeHandle() const
		{
			return m_MaterialDataGPU.GetNativeHandle();
		}

		void Advance()
		{
			m_MaterialDataGPU.Advance();
		}

	private:
		stdext::flat_map<MaterialKey, MaterialData> m_MaterialDataCPU;
		MultiBuffer<PersistentBuffer> m_MaterialDataGPU;
	};

}
