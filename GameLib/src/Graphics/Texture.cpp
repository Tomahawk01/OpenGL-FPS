#include "Texture.h"

#include "Utils/Error.h"

namespace {

	GLenum ToOpenGL(Game::TextureFormat format, bool includeSize)
	{
		switch (format)
		{
			case Game::TextureFormat::RGB: return includeSize ? GL_RGB8 : GL_RGB;
			case Game::TextureFormat::RGBA: return includeSize ? GL_RGBA8 : GL_RGBA;
		}
		throw Game::Exception("Unknown texture format: {}", format);
	}

}

namespace Game {

	Texture::Texture(const TextureData& texture, const std::string& name, const Sampler& sampler)
		: m_Handle{ 0u, [](auto texture) { glDeleteTextures(1, &texture); } }
		, m_BindlessHandle{}
		, m_Name{ name }
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_Handle);
		glObjectLabel(GL_TEXTURE, m_Handle, name.length(), name.data());
		glTextureStorage2D(m_Handle, 1, ToOpenGL(texture.format, true), texture.width, texture.height);
		glTextureSubImage2D(m_Handle, 0, 0, 0, texture.width, texture.height, ToOpenGL(texture.format, false), GL_UNSIGNED_BYTE, texture.data.data());

		m_BindlessHandle = glGetTextureSamplerHandleARB(m_Handle, sampler.GetNativeHandle());
		glMakeTextureHandleResidentARB(m_BindlessHandle);
	}

	Texture::~Texture()
	{
		glMakeTextureHandleNonResidentARB(m_BindlessHandle);
	}

	GLuint64 Texture::GetNativeHandle() const
	{
		return m_BindlessHandle;
	}

	std::string Texture::GetName() const
	{
		return m_Name;
	}

}
