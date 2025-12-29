#pragma once

#include "TextureData.h"
#include "Utils/DataBuffer.h"
#include "Utils/Log.h"
#include "OpenGL.h"

#include <vector>
#include <string_view>

namespace Game {

	template<class T>
	concept IsBuffer = requires(T t, DataBufferView data, size_t offset)
	{
		{ t.Write(data, offset) };
	};

	template<class T, IsBuffer Buffer>
	void ResizeGPUBuffer(const std::vector<T>& cpuBuffer, Buffer& gpuBuffer, std::string_view name)
	{
		const auto bufferSizeBytes = cpuBuffer.size() * sizeof(T);
		if (gpuBuffer.GetSize() <= bufferSizeBytes)
		{
			auto newSize = gpuBuffer.GetSize() * 2;
			while (newSize < bufferSizeBytes)
			{
				newSize *= 2;
			}

			Game::Log::Info("Growing {} buffer {} -> {}", name, gpuBuffer.GetSize(), newSize);

			// OpenGL barrier in case gpu using previous frame
			glFinish();

			gpuBuffer = Buffer{ newSize, name };
		}
	}

	TextureData LoadTexture(DataBufferView imageData);

}
