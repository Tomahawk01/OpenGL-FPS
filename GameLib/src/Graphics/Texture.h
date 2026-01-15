#pragma once

#include "TextureData.h"
#include "Sampler.h"

#include <string>

namespace Game {

	class Texture
	{
	public:
		Texture(const TextureData& texture, const std::string& name, const Sampler& sampler);
		~Texture();

		Texture(Texture&&) = default;
		Texture& operator=(Texture&&) = default;

		GLuint GetNativeHandle() const;
		GLuint64 GetBindlessHandle() const;
		std::string GetName() const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;

	private:
		AutoRelease<GLuint> m_Handle;
		GLuint64 m_BindlessHandle;
		std::string m_Name;
		uint32_t m_Width;
		uint32_t m_Height;
	};

}
