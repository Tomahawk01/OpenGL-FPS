#pragma once

#include "Utils/DataBuffer.h"

#include <string>

namespace Game {

	enum class TextureFormat
	{
		RED,
		RGB,
		RGBA
	};

	struct TextureData
	{
		uint32_t width;
		uint32_t height;
		TextureFormat format;
		DataBuffer data;
	};

	inline std::string to_string(TextureFormat format)
	{
		switch (format)
		{
			case Game::TextureFormat::RED: return "RED";
			case Game::TextureFormat::RGB: return "RGB";
			case Game::TextureFormat::RGBA: return "RGBA";
			default: return "unknown";
		}
	}

}
