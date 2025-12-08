#pragma once

#include "Buffer.h"
#include "MeshView.h"
#include "VertexData.h"

#include <vector>
#include <string>

namespace Game {

	class MeshManager
	{
	public:
		MeshManager();

		MeshView Load(const std::vector<VertexData>& mesh);

		GLuint GetNativeHandle() const;
		std::string to_string() const;

	private:
		std::vector<VertexData> m_MeshDataCPU;
		Buffer m_MeshDataGPU;
	};

}
