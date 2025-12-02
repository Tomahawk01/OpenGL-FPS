#pragma once

#include <vector>
#include <span>
#include <cstddef>

namespace Game {

	using DataBuffer = std::vector<std::byte>;
	using DataBufferView = std::span<const std::byte>;

}
