#pragma once

#include "OpenGL.h"
#include "Shader.h"
#include "Utils/AutoRelease.h"

#include <string_view>

namespace Game {

	class Program
	{
	public:
		Program(const Shader& vertexShader, const Shader& fragmentShader, std::string_view name);

		void Use() const;

		GLuint GetNativeHandle() const;

	private:
		AutoRelease<GLuint> m_Handle;
	};

}
