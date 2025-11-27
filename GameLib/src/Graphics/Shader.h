#pragma once

#include "OpenGL.h"
#include "Utils/AutoRelease.h"

#include <string>
#include <string_view>

namespace Game {

	enum class ShaderType
	{
		VERTEX,
		FRAGMENT
	};

	class Shader
	{
	public:
		Shader(std::string_view source, ShaderType type, std::string_view name);

		ShaderType GetType() const;
		GLuint GetNativeHandle() const;

	private:
		AutoRelease<GLuint> m_Handle;
		ShaderType m_Type;
	};

	std::string to_string(ShaderType obj);

}
