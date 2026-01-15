#pragma once

#include "Utils/AutoRelease.h"
#include "Texture.h"
#include "OpenGL.h"

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace Game {

	class FrameBuffer
	{
	public:
		FrameBuffer(std::vector<const Texture*> colorTextures, const Texture* depthTexture, const std::string& name);

		void Bind() const;
		void UnBind() const;

		uint32_t GetWidth() const;
		uint32_t GetHeight() const;

		GLuint GetNativeHandle() const;
		std::span<const Texture* const> GetColorTextures() const;
		std::string_view GetName() const;

	private:
		AutoRelease<GLuint> m_Handle;
		std::vector<const Texture*> m_ColorTextures;
		const Texture* m_DepthTexture;
		std::string m_Name;
	};

}
