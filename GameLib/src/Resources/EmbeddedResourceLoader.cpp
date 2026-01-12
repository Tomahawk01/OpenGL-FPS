#include "EmbeddedResourceLoader.h"

#include "Utils/Error.h"

#include <algorithm>
#include <cstdint>
#include <cstddef>

namespace {

	constexpr const char sampleVertexShader[] = {
		#embed "../Game/assets/shaders/simple.vert"
	};

	constexpr const char sampleFragmentShader[] = {
		#embed "../Game/assets/shaders/simple.frag"
	};

	constexpr const char diamondFloorAlbedo[] = {
		#embed "../Game/assets/textures/diamond_floor_albedo.png"
	};

	constexpr const char diamondFloorNormal[] = {
		#embed "../Game/assets/textures/diamond_floor_normal.png"
	};

	constexpr const char diamondFloorSpecular[] = {
		#embed "../Game/assets/textures/diamond_floor_specular.png"
	};

	template<class T>
	T ToContainer(std::span<const char> data)
	{
		static_assert(sizeof(typename T::value_type) == 1);

		const auto* ptr = reinterpret_cast<const T::value_type*>(data.data());
		return T{ ptr, ptr + data.size() };
	}

}

namespace Game {

	EmbeddedResourceLoader::EmbeddedResourceLoader()
	{
		m_Lookup = {
			{"shaders\\simple.vert", sampleVertexShader},
			{"shaders\\simple.frag", sampleFragmentShader},
			{"textures\\diamond_floor_albedo.png", diamondFloorAlbedo},
			{"textures\\diamond_floor_normal.png", diamondFloorNormal},
			{"textures\\diamond_floor_specular.png", diamondFloorSpecular},
		};
	}

	std::string EmbeddedResourceLoader::_LoadString(std::string_view name)
	{
		const auto resource = m_Lookup.find(name);
		Expect(resource != std::ranges::cend(m_Lookup), "Resource {} does not exist", name);

		return ToContainer<std::string>(resource->second);
	}

	DataBuffer EmbeddedResourceLoader::LoadDataBuffer(std::string_view name)
	{
		const auto resource = m_Lookup.find(name);
		Expect(resource != std::ranges::cend(m_Lookup), "Resource {} does not exist", name);

		return ToContainer<DataBuffer>(resource->second);
	}

}
