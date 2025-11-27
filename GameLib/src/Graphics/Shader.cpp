#include "Shader.h"

#include "Utils/Error.h"
#include "Utils/Exception.h"

#include <format>

namespace {

	GLenum ToNative(Game::ShaderType type)
	{
		switch (type)
		{
			case Game::ShaderType::VERTEX: return GL_VERTEX_SHADER;
			case Game::ShaderType::FRAGMENT: return GL_FRAGMENT_SHADER;
		}

		throw Game::Exception("Unknown shader type: {}", std::to_underlying(type));
	}

}

namespace Game {

	Shader::Shader(std::string_view source, ShaderType type, std::string_view name)
		: m_Handle{}
		, m_Type(type)
	{
		m_Handle = AutoRelease<GLuint>{
			glCreateShader(ToNative(type)),
			glDeleteShader
		};
		glObjectLabel(GL_SHADER, m_Handle, name.length(), name.data());

		const GLchar* strings[] = { source.data() };
		const GLint lengths[] = { static_cast<GLint>(source.length()) };

		glShaderSource(m_Handle, 1, strings, lengths);
		glCompileShader(m_Handle);

		GLint result{};
		glGetShaderiv(m_Handle, GL_COMPILE_STATUS, &result);

		if (result != GL_TRUE)
		{
			char log[512];
			glGetShaderInfoLog(m_Handle, sizeof(log), nullptr, log);

			Ensure(result, "Failed to compile shader {} {}\n{}", m_Type, name, log);
		}
	}

	ShaderType Shader::GetType() const
	{
		return m_Type;
	}

	GLuint Shader::GetNativeHandle() const
	{
		return m_Handle;
	}

	std::string to_string(ShaderType obj)
	{
		switch (obj)
		{
			case ShaderType::VERTEX: return "VERTEX";
			case ShaderType::FRAGMENT: return "FRAGMENT";
		}

		throw Exception("Unknown shader type: {}", std::to_underlying(obj));
	}

}
