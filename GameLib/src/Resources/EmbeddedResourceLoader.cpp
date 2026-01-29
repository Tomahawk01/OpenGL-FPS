#include "EmbeddedResourceLoader.h"

#include "Utils/Error.h"

#include <algorithm>
#include <cstdint>
#include <cstddef>

namespace {

	constexpr const char simpleVertexShader[] = {
		#embed "../Game/assets/shaders/simple.vert"
	};

	constexpr const char simpleFragmentShader[] = {
		#embed "../Game/assets/shaders/simple.frag"
	};

	constexpr const char gbufferVertexShader[] = {
		#embed "../Game/assets/shaders/gbuffer.vert"
	};

	constexpr const char gbufferFragmentShader[] = {
		#embed "../Game/assets/shaders/gbuffer.frag"
	};

	constexpr const char lightPassVertexShader[] = {
		#embed "../Game/assets/shaders/light_pass.vert"
	};

	constexpr const char lightPassFragmentShader[] = {
		#embed "../Game/assets/shaders/light_pass.frag"
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

	constexpr const char de_dust2[] = {
		#embed "../Game/assets/models/de_dust2.glb"
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
			{"models\\de_dust2.glb", de_dust2},

			{"shaders\\simple.vert", simpleVertexShader},
			{"shaders\\simple.frag", simpleFragmentShader},
			{"shaders\\gbuffer.vert", gbufferVertexShader},
			{"shaders\\gbuffer.frag", gbufferFragmentShader},
			{"shaders\\light_pass.vert", lightPassVertexShader},
			{"shaders\\light_pass.frag", lightPassFragmentShader},

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
