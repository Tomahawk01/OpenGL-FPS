#include "Sampler.h"

#include "Utils/Error.h"
#include "Utils/Exception.h"
#include "Utils/Formatter.h"

namespace {

	GLenum ToOpenGL(Game::FilterType filterType)
	{
		switch (filterType)
		{
			case Game::FilterType::LINEAR_MIPMAP: return GL_LINEAR_MIPMAP_LINEAR;
			case Game::FilterType::LINEAR: return GL_LINEAR;
			case Game::FilterType::NEAREST: return GL_NEAREST;
		}

		throw Game::Exception("Unknown filerType: {}", filterType);
	}

}

namespace Game {

	Sampler::Sampler(FilterType minFilter, FilterType magFilter, const std::string& name, std::optional<float> anisotropySamples)
		: m_Handle{ 0u, [](auto sampler) { glDeleteSamplers(1, &sampler); } }
		, m_Name{ name }
	{
		glCreateSamplers(1, &m_Handle);
		glObjectLabel(GL_SAMPLER, m_Handle, name.length(), name.data());

		glSamplerParameteri(m_Handle, GL_TEXTURE_MIN_FILTER, ToOpenGL(minFilter));
		glSamplerParameteri(m_Handle, GL_TEXTURE_MAG_FILTER, ToOpenGL(magFilter));

		if (anisotropySamples)
		{
			Expect(*anisotropySamples >= 1.0f, "Invalid samples: {}", *anisotropySamples);
			glSamplerParameterf(m_Handle, GL_TEXTURE_MAX_ANISOTROPY_EXT, *anisotropySamples);
		}
	}

	::GLuint Sampler::GetNativeHandle() const
	{
		return m_Handle;
	}

	std::string Sampler::GetName() const
	{
		return m_Name;
	}

}
