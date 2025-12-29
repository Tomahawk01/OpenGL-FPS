#pragma once

#include "Utils/DataBuffer.h"

namespace Game {

	enum class TextureFormat
	{
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

}
