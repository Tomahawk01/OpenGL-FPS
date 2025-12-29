#include "Utils.h"

#include "Utils/Error.h"

#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace {

	Game::TextureFormat ChannelsToFormat(int numChannels)
	{
		switch (numChannels)
		{
			case 3: return Game::TextureFormat::RGB;
			case 4: return Game::TextureFormat::RGBA;
		}

		throw Game::Exception("Unsupported channel count: {}", numChannels);
	}

}

namespace Game {

	TextureData LoadTexture(DataBufferView imageData)
	{
		int width{};
		int height{};
		int numChannels{};

		auto rawData = std::unique_ptr<stbi_uc, void(*)(void*)>{
			stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(imageData.data()), imageData.size(), &width, &height, &numChannels, 0),
			stbi_image_free
		};
		Ensure(rawData, "Failed to parse texture data");

		const auto* ptr = reinterpret_cast<const std::byte*>(rawData.get());

		return {
			.width = static_cast<uint32_t>(width),
			.height = static_cast<uint32_t>(height),
			.format = ChannelsToFormat(numChannels),
			.data = { ptr, ptr + width * height * numChannels }
		};
	}

}
