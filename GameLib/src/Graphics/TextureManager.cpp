#include "TextureManager.h"

#include "Utils.h"
#include "Utils\Log.h"

#include <span>
#include <ranges>

namespace Game {

	TextureManager::TextureManager()
		: m_GPUBuffer{ sizeof(GLuint64), "bindless_textures" }
		, m_CPUBuffer{}
		, m_Textures{}
	{}

	uint32_t TextureManager::Add(Texture texture)
	{
		const auto newIndex = m_Textures.size();

		auto& newTex = m_Textures.emplace_back(std::move(texture));
		m_CPUBuffer.push_back(newTex.GetNativeHandle());

		ResizeGPUBuffer(m_CPUBuffer, m_GPUBuffer, "bindless_textures");

		m_GPUBuffer.Write(std::as_bytes(std::span{ m_CPUBuffer.data(), m_CPUBuffer.size() }), 0zu);

		return newIndex;
	}

	uint32_t TextureManager::Add(std::vector<Texture> textures)
	{
		const auto newIndex = m_Textures.size();

		for (const auto& t : textures)
		{
			Log::Trace("{}", t.GetNativeHandle());
		}

		m_Textures.append_range(std::views::as_rvalue(textures));
		m_CPUBuffer = m_Textures | std::views::transform([](auto& e) { return e.GetNativeHandle(); }) | std::ranges::to<std::vector>();

		for (const auto& t : m_CPUBuffer)
		{
			Log::Trace("{}", t);
		}

		ResizeGPUBuffer(m_CPUBuffer, m_GPUBuffer, "bindless_textures");

		m_GPUBuffer.Write(std::as_bytes(std::span{ m_CPUBuffer.data(), m_CPUBuffer.size() }), 0zu);

		return newIndex;
	}

	GLuint TextureManager::GetNativeHandle() const
	{
		return m_GPUBuffer.GetNativeHandle();
	}

}
