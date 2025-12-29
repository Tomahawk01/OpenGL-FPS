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

		GLuint64 GetNativeHandle() const;
		std::string GetName() const;

	private:
		AutoRelease<GLuint> m_Handle;
		GLuint64 m_BindlessHandle;
		std::string m_Name;
	};

}
