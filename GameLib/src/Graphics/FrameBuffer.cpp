#include "FrameBuffer.h"

#include "Utils/Error.h"

#include <algorithm>
#include <ranges>

namespace Game {

	FrameBuffer::FrameBuffer(std::vector<const Texture*> colorTextures, const Texture* depthTexture, const std::string& name)
		: m_Handle(0u, [](const auto buffer) { glDeleteFramebuffers(1u, &buffer); })
		, m_ColorTextures{ colorTextures }
		, m_DepthTexture{ depthTexture }
		, m_Name{ name }
	{
		Expect(!m_ColorTextures.empty(), "Must have color textures");
		Expect(m_ColorTextures.size() < 8u, "Hit arbitrary color texture limit");
		Expect(std::ranges::all_of(m_ColorTextures,
								   [&](const auto* e)
								   {
									   return e->GetWidth() == m_ColorTextures[0]->GetWidth() &&
											  e->GetHeight() == m_ColorTextures[0]->GetHeight();
								   }), "All color textures must have same dimensions");

		glCreateFramebuffers(1, &m_Handle);

		for (const auto& [index, colorTex] : std::views::enumerate(m_ColorTextures))
		{
			glNamedFramebufferTexture(m_Handle, static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + index), colorTex->GetNativeHandle(), 0);
		}

		glNamedFramebufferTexture(m_Handle, GL_DEPTH_ATTACHMENT, m_DepthTexture->GetNativeHandle(), 0);

		const auto attachments = std::views::iota(size_t{ 0 }, m_ColorTextures.size()) |
			std::views::transform([](auto e) { return static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + e); }) |
			std::ranges::to<std::vector>();

		glNamedFramebufferDrawBuffers(m_Handle, static_cast<GLsizei>(attachments.size()), attachments.data());

		Expect(glCheckNamedFramebufferStatus(m_Handle, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not complete");

		glObjectLabel(GL_FRAMEBUFFER, m_Handle, name.length(), name.data());
	}

	void FrameBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Handle);
	}

	void FrameBuffer::UnBind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0u);
	}

	uint32_t FrameBuffer::GetWidth() const
	{
		return m_ColorTextures.front()->GetWidth();
	}

	uint32_t FrameBuffer::GetHeight() const
	{
		return m_ColorTextures.front()->GetHeight();
	}

	GLuint FrameBuffer::GetNativeHandle() const
	{
		return m_Handle;
	}

	std::span<const Texture* const> FrameBuffer::GetColorTextures() const
	{
		return m_ColorTextures;
	}

	std::string_view FrameBuffer::GetName() const
	{
		return m_Name;
	}

}
