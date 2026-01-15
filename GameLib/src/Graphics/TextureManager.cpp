#include "TextureManager.h"

#include "Utils.h"
#include "Utils\Error.h"
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
		m_CPUBuffer.push_back(newTex.GetBindlessHandle());

		ResizeGPUBuffer(m_CPUBuffer, m_GPUBuffer, "bindless_textures");

		m_GPUBuffer.Write(std::as_bytes(std::span{ m_CPUBuffer.data(), m_CPUBuffer.size() }), 0zu);

		return newIndex;
	}

	uint32_t TextureManager::Add(std::vector<Texture> textures)
	{
		const auto newIndex = m_Textures.size();

		m_Textures.append_range(std::views::as_rvalue(textures));
		m_CPUBuffer = m_Textures | std::views::transform([](auto& e) { return e.GetBindlessHandle(); }) | std::ranges::to<std::vector>();

		ResizeGPUBuffer(m_CPUBuffer, m_GPUBuffer, "bindless_textures");

		m_GPUBuffer.Write(std::as_bytes(std::span{ m_CPUBuffer.data(), m_CPUBuffer.size() }), 0zu);

		return newIndex;
	}

	GLuint TextureManager::GetNativeHandle() const
	{
		return m_GPUBuffer.GetNativeHandle();
	}

	const Texture* TextureManager::GetTexture(uint32_t index) const
	{
		Expect(index <= m_Textures.size(), "index {} out of range", index);
		return std::addressof(m_Textures[index]);
	}

	std::vector<const Texture*> TextureManager::GetTextures(const std::vector<uint32_t>& indices) const
	{
		return indices |
			std::views::transform(
				[this](auto i) {
					Expect(i <= m_Textures.size(), "index {} out of range", i);
					return std::addressof(m_Textures[i]);
				}) |
			std::ranges::to<std::vector>();
	}

}
