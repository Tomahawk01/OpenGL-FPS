#include "FileResourceLoader.h"

#include "Utils/AutoRelease.h"
#include "Utils/Error.h"
#include "Utils/Log.h"

#include <Windows.h>
#undef LoadString

#include <cstddef>

namespace {

	auto Init(const std::filesystem::path& path)
	{
		Game::AutoRelease<HANDLE, nullptr> handle{
			CreateFileA(path.string().c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr),
			CloseHandle
		};
		Game::Ensure(handle.Get() != INVALID_HANDLE_VALUE, "Failed to open file: {}, error code: {}", path.string(), GetLastError());

		Game::AutoRelease<HANDLE, nullptr> mapping{
			CreateFileMappingA(handle, nullptr, PAGE_READONLY, 0, 0, nullptr),
			CloseHandle
		};
		Game::Ensure(mapping, "Failed to map file: {}, error code {}", path.string(), GetLastError());

		std::unique_ptr<void, decltype(&UnmapViewOfFile)> mapView{
			MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0),
			UnmapViewOfFile
		};
		Game::Ensure(mapView, "Failed to get map view: {}, error code: {}", path.string(), GetLastError());

		return std::make_tuple(std::move(handle), std::move(mapping), std::move(mapView));
	}

	template<class T>
	auto Load(const std::filesystem::path& path)
	{
		static_assert(sizeof(typename T::value_type) == 1);

		const auto& [handle, mapping, mapView] = Init(path);
		const auto size = GetFileSize(handle, nullptr);
		const auto* ptr = reinterpret_cast<T::value_type*>(mapView.get());

		Game::Log::Trace("Loaded resource: {} ({})", path.string(), size);

		return T{ ptr, ptr + size };
	}

}

namespace Game {

	FileResourceLoader::FileResourceLoader(const std::filesystem::path& root)
		: m_Root{ root }
	{}

	std::string FileResourceLoader::LoadString(std::string_view name)
	{
		return Load<std::string>(m_Root / name);
	}

	DataBuffer FileResourceLoader::LoadDataBuffer(std::string_view name)
	{
		return Load<DataBuffer>(m_Root / name);
	}

}
