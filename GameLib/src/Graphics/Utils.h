#pragma once

#include "TextureData.h"
#include "VertexData.h"
#include "Utils/DataBuffer.h"
#include "Utils/Log.h"
#include "Resources/ResourceLoader.h"
#include "OpenGL.h"

#include "ModelData.h"

#include <vector>
#include <ranges>
#include <string_view>
#include <type_traits>

namespace Game {

	template<class T>
	concept IsBuffer = requires(T t, DataBufferView data, size_t offset)
	{
		{ t.Write(data, offset) };
		{ t.GetName() } -> std::convertible_to<std::string_view>;
	};

	template<class ...Args>
	std::vector<Game::VertexData> Vertices(Args&&... args)
	{
		return std::views::zip_transform(
			[]<class ...A>(A&&... a) { return Game::VertexData{ std::forward<A>(a)... }; },
			std::forward<Args>(args)...
		) | std::ranges::to<std::vector>();
	}

	template<class T, IsBuffer Buffer>
	void ResizeGPUBuffer(const std::vector<T>& cpuBuffer, Buffer& gpuBuffer)
	{
		const auto bufferSizeBytes = cpuBuffer.size() * sizeof(T);
		if (gpuBuffer.GetSize() <= bufferSizeBytes)
		{
			auto newSize = gpuBuffer.GetSize() * 2zu;
			while (newSize < bufferSizeBytes)
			{
				newSize *= 2zu;
			}

			Game::Log::Info("Growing {} buffer {} -> {}", gpuBuffer.GetName(), gpuBuffer.GetSize(), newSize);

			// OpenGL barrier in case gpu using previous frame
			glFinish();

			gpuBuffer = Buffer{ newSize, gpuBuffer.GetName() };
		}
	}

	TextureData LoadTexture(DataBufferView imageData);
	std::vector<ModelData> LoadModel(DataBufferView modelData, ResourceLoader& resourceLoader);

}
