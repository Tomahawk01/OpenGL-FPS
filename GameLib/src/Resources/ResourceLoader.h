#pragma once

#include "Utils/DataBuffer.h"

#include <string_view>

namespace Game {

	class ResourceLoader
	{
	public:
		virtual ~ResourceLoader() = default;

		virtual std::string _LoadString(std::string_view name) = 0;
		virtual DataBuffer LoadDataBuffer(std::string_view name) = 0;
	};

}
