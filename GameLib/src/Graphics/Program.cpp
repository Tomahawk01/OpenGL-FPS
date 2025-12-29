#include "Program.h"

#include "Utils/Error.h"

namespace {

	void CheckState(GLuint handle, GLenum state, std::string_view name, std::string_view message)
	{
		auto res = GLint{};
		glGetProgramiv(handle, state, &res);
		if (res != GL_TRUE)
		{
			char log[512]{};
			glGetProgramInfoLog(handle, sizeof(log), nullptr, log);
			throw Game::Exception("{}: {} {}", message, name, log);
		}
	}

}

namespace Game {

	Program::Program(const Shader& vertexShader, const Shader& fragmentShader, std::string_view name)
		: m_Handle{}
	{
		Expect(vertexShader.GetType() == ShaderType::VERTEX, "Shader is not a vertex shader");
		Expect(fragmentShader.GetType() == ShaderType::FRAGMENT, "Shader is not a fragment shader");

		m_Handle = { glCreateProgram(), glDeleteProgram };
		Ensure(m_Handle, "Failed to create OpenGL program");

		glObjectLabel(GL_PROGRAM, m_Handle, name.length(), name.data());

		glAttachShader(m_Handle, vertexShader.GetNativeHandle());
		glAttachShader(m_Handle, fragmentShader.GetNativeHandle());
		glLinkProgram(m_Handle);
		glValidateProgram(m_Handle);

		CheckState(m_Handle, GL_LINK_STATUS, name, "Failed to link program");
		CheckState(m_Handle, GL_VALIDATE_STATUS, name, "Failed to validate program");
	}

	void Program::Use() const
	{
		glUseProgram(m_Handle);
	}

	GLuint Program::GetNativeHandle() const
	{
		return m_Handle;
	}

}
