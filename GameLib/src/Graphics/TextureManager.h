#pragma once

#include "Buffer.h"
#include "OpenGL.h"
#include "Texture.h"

#include <cstdint>
#include <vector>

namespace Game {

    class TextureManager
    {
    public:
        TextureManager();

        uint32_t Add(Texture texture);
        uint32_t Add(std::vector<Texture> textures);

        GLuint GetNativeHandle() const;

    private:
        Buffer m_GPUBuffer;
        std::vector<GLuint64> m_CPUBuffer;
        std::vector<Texture> m_Textures;
    };

}
