#pragma once

#include "Utils/AutoRelease.h"
#include "OpenGL.h"

#include <string>
#include <optional>

namespace Game {

	enum class FilterType
	{
		LINEAR_MIPMAP,
		LINEAR,
		NEAREST
	};

	class Sampler
	{
	public:
		Sampler(FilterType minFilter, FilterType magFilter, const std::string& name, std::optional<float> anisotropySamples = std::nullopt);

		GLuint GetNativeHandle() const;
		std::string GetName() const;

	private:
		AutoRelease<GLuint> m_Handle;
		std::string m_Name;
	};

	inline std::string to_string(FilterType filterType)
	{
		switch (filterType)
		{
			case FilterType::LINEAR_MIPMAP: return "LINEAR_MIPMAP";
			case FilterType::LINEAR: return "LINEAR";
			case FilterType::NEAREST: return "NEAREST";
			default: return "unknown";
		}
	}

}
