#pragma once

#include "ResourceLoader.h"

#include <filesystem>

namespace Game {

	class FileResourceLoader : public ResourceLoader
	{
	public:
		FileResourceLoader(const std::filesystem::path& root);
		~FileResourceLoader() override = default;

		std::string _LoadString(std::string_view name) override;
		DataBuffer LoadDataBuffer(std::string_view name) override;

	private:
		std::filesystem::path m_Root;
	};

}
