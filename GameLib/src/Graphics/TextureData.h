#pragma once

#include "Utils/DataBuffer.h"

#include <optional>
#include <string>

namespace Game {

	enum class TextureFormat
	{
		RED,
		RGB,
		RGBA,
		RGB16F,
		DEPTH24
	};

	struct TextureData
	{
		uint32_t width;
		uint32_t height;
		TextureFormat format;
		std::optional<DataBuffer> data;
	};

	inline std::string to_string(TextureFormat format)
	{
		switch (format)
		{
			case TextureFormat::RED: return "RED";
			case TextureFormat::RGB: return "RGB";
			case TextureFormat::RGBA: return "RGBA";
			case TextureFormat::RGB16F: return "RGB16F";
			case TextureFormat::DEPTH24: return "DEPTH24";
			default: return "unknown";
		}
	}

}
