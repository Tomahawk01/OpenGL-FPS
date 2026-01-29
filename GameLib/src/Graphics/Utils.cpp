#include "Utils.h"

#include "Utils/Error.h"
#include "Utils/Log.h"

#include <filesystem>
#include <memory>
#include <span>

#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/Logger.hpp>
#include <assimp/scene.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace {

	template<Game::Log::Level L>
	class SimpleAssimpLogStream : public Assimp::LogStream
	{
	public:
		void write(const char* msg) override
		{
			if constexpr (L == Game::Log::Level::INFO)
			{
				Game::Log::Info("{}", msg);
			}
			else if constexpr (L == Game::Log::Level::WARN)
			{
				Game::Log::Warn("{}", msg);
			}
			else if constexpr (L == Game::Log::Level::ERR)
			{
				Game::Log::Error("{}", msg);
			}
			else if constexpr (L == Game::Log::Level::TRACE)
			{
				Game::Log::Trace("{}", msg);
			}
			else
			{
				Game::Log::Error("[Unknown Level] {}", msg);
			}
		}
	};

	Game::vec3 ToNative(const aiVector3D& v)
	{
		return { v.x, v.y, v.z };
	}

	Game::TextureFormat ChannelsToFormat(int numChannels)
	{
		switch (numChannels)
		{
			case 1: return Game::TextureFormat::RED;
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

		stbi_set_flip_vertically_on_load(true);
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
			.data = { {ptr, ptr + width * height * numChannels} }
		};
	}

	std::vector<ModelData> LoadModel(DataBufferView modelData, ResourceLoader& resourceLoader)
	{
		[[maybe_unused]] static auto* logger = []
		{
			Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
			auto* logger = Assimp::DefaultLogger::get();

			logger->attachStream(new SimpleAssimpLogStream<Log::Level::ERR>{}, Assimp::Logger::Err);
			logger->attachStream(new SimpleAssimpLogStream<Log::Level::TRACE>{}, Assimp::Logger::Debugging);
			logger->attachStream(new SimpleAssimpLogStream<Log::Level::WARN>{}, Assimp::Logger::Warn);
			logger->attachStream(new SimpleAssimpLogStream<Log::Level::INFO>{}, Assimp::Logger::Info);

			return logger;
		}();

		auto importer = Assimp::Importer{};
		const auto* scene = importer.ReadFileFromMemory(modelData.data(), modelData.size(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		Ensure(scene != nullptr, "Failed to parse assimp scene");

		const auto loadedMeshes = std::span<aiMesh*>(scene->mMeshes, scene->mMeshes + scene->mNumMeshes);
		const auto materials = std::span<aiMaterial*>(scene->mMaterials, scene->mMaterials + scene->mNumMaterials);
		Log::Info("Found {} meshes, {} materials", std::ranges::size(loadedMeshes), std::ranges::size(materials));

		//Ensure(std::ranges::size(loadedMeshes) == std::ranges::size(materials), "Mismatch mesh/material count in model file");

		auto models = std::vector<ModelData>{};

		for (const auto& [index, mesh] : loadedMeshes | std::views::enumerate)
		{
			Log::Info("Found mesh: {}", mesh->mName.C_Str());

			const auto* material = scene->mMaterials[index];
			const auto baseColorCount = material->GetTextureCount(aiTextureType_BASE_COLOR);
			if (baseColorCount != 1)
			{
				Log::Warn("Unsupported base color count: {}", baseColorCount);
				continue;
			}

			auto pathStr = aiString{};
			material->GetTexture(aiTextureType_BASE_COLOR, 0u, &pathStr);
			const auto path = std::filesystem::path{ pathStr.C_Str() };
			const auto filename = path.filename();
			Log::Info("Found base color texture: {}", filename.string());

			const auto positions = std::span<aiVector3D>{ mesh->mVertices, mesh->mVertices + mesh->mNumVertices } | std::views::transform(ToNative);
			const auto normals = std::span<aiVector3D>{ mesh->mNormals, mesh->mNormals + mesh->mNumVertices } | std::views::transform(ToNative);
			const auto tangents = std::span<aiVector3D>{ mesh->mTangents, mesh->mTangents + mesh->mNumVertices } | std::views::transform(ToNative);
			const auto bitangents = std::span<aiVector3D>{ mesh->mBitangents, mesh->mBitangents + mesh->mNumVertices } | std::views::transform(ToNative);
			const auto uvs = std::span<aiVector3D>{ mesh->mTextureCoords[0], mesh->mTextureCoords[0] + mesh->mNumVertices } |
				std::views::transform([](const auto& v) { return UV{ .s = v.x, .t = v.y }; });

			auto indices = std::span<aiFace>{ mesh->mFaces, mesh->mFaces + mesh->mNumFaces } |
				std::views::transform([](const auto& e) { return std::span<uint32_t>{ e.mIndices, e.mIndices + e.mNumIndices }; }) |
				std::views::join |
				std::ranges::to<std::vector>();

			models.push_back({
				.meshData = {
					.vertices = Vertices(positions, normals, tangents, bitangents, uvs),
					.indices = std::move(indices)
				},
				.albedo = std::nullopt, //LoadTexture(resourceLoader.LoadDataBuffer(std::format("textures\\{}", filename.string()))),
				.normal = std::nullopt,
				.specular = std::nullopt
			});
		}

		return models;
	}

}
