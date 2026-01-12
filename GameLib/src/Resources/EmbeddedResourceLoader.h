#pragma once

#include "ResourceLoader.h"
#include "Utils/StringMap.h"

#include <span>

namespace Game {

	class EmbeddedResourceLoader : public ResourceLoader
	{
	public:
		EmbeddedResourceLoader();
		~EmbeddedResourceLoader() override = default;

		std::string _LoadString(std::string_view name) override;
		DataBuffer LoadDataBuffer(std::string_view name) override;

	private:
		StringMap<std::span<const char>> m_Lookup;
	};

}
