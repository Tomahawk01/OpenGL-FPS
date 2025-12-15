#include "Renderer.h"

#include "ObjectData.h"
#include "Utils.h"

#include <string_view>
#include <ranges>
#include <span>

using namespace std::literals;

namespace {

	constexpr auto sampleVertexShader = R"(
	#version 460 core
	
	struct VertexData
	{
		float position[3];
	};

	struct ObjectData
	{
		mat4 model;
		uint material_index;
	};

	struct MaterialData
	{
		float color[3];
	};

	layout(binding = 0, std430) readonly buffer vertices
	{
		VertexData data[];
	};

	layout(binding = 1, std430) readonly buffer camera
	{
		mat4 view;
		mat4 projection;
	};

	layout(binding = 2, std430) readonly buffer objects
	{
		ObjectData objectData[];
	};

	layout(binding = 3, std430) readonly buffer materials
	{
		MaterialData materialData[];
	};

	vec3 get_position(uint index)
	{
		return vec3(data[index].position[0], data[index].position[1], data[index].position[2]);
	}

	layout(location = 0) out flat uint material_index;

	void main()
	{
		gl_Position = projection * view * objectData[gl_DrawID].model * vec4(get_position(gl_VertexID), 1.0);
		material_index = objectData[gl_DrawID].material_index;
	}
	)"sv;

	constexpr auto sampleFragmentShader = R"(
	#version 460 core
	
	struct VertexData
	{
		float position[3];
	};

	struct ObjectData
	{
		mat4 model;
		uint material_index;
	};

	struct MaterialData
	{
		float color[3];
	};

	layout(binding = 0, std430) readonly buffer vertices
	{
		VertexData data[];
	};

	layout(binding = 1, std430) readonly buffer camera
	{
		mat4 view;
		mat4 projection;
	};

	layout(binding = 2, std430) readonly buffer objects
	{
		ObjectData objectData[];
	};

	layout(binding = 3, std430) readonly buffer materials
	{
		MaterialData materialData[];
	};

	vec3 get_color(uint index)
	{
		return vec3(materialData[index].color[0], materialData[index].color[1], materialData[index].color[2]);
	}

	layout(location = 0) in flat uint material_index;

	layout(location = 0) out vec4 out_color;

	void main()
	{
		out_color = vec4(get_color(material_index), 1.0);
	}
	)"sv;

	Game::Program CreateProgram()
	{
		const auto sampleVert = Game::Shader{ sampleVertexShader, Game::ShaderType::VERTEX, "sample_vertex_shader"sv };
		const auto sampleFrag = Game::Shader{ sampleFragmentShader, Game::ShaderType::FRAGMENT, "sample_fragment_shader"sv };
		return { sampleVert, sampleFrag, "sample_prog"sv };
	}

}

namespace Game {

	Renderer::Renderer()
		: m_DummyVAO{ 0u, [](auto e) { glDeleteVertexArrays(1, &e); } }
		, m_CommandBuffer{}
		, m_CameraBuffer{ sizeof(CameraData), "camera_buffer" }
		, m_ObjectDataBuffer{ sizeof(ObjectData), "object_data_buffer" }
		, m_Program{ CreateProgram() }
	{
		glGenVertexArrays(1, &m_DummyVAO);
		glBindVertexArray(m_DummyVAO);

		m_Program.Use();
	}

	void Renderer::Render(const Scene& scene)
	{
		m_CameraBuffer.Write(scene.camera.GetDataView(), 0);
		const auto [vertexBufferHandle, indexBufferHandle] = scene.meshManager.GetNativeHandle();
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexBufferHandle);
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, m_CameraBuffer.GetNativeHandle(), m_CameraBuffer.FrameOffsetBytes(), sizeof(CameraData));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle);

		const auto commandCount = m_CommandBuffer.Build(scene);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_CommandBuffer.GetNativeHandle());

		const auto objectData = scene.entities |
								std::views::transform([&scene](const auto& e)
													  {
														  const auto index = scene.materialManager.Index(e.materialKey);
														  return ObjectData{ .model = e.transform, .materialIDIndex = index, .padding = {} };
													  }) |
								std::ranges::to<std::vector>();
		ResizeGPUBuffer(objectData, m_ObjectDataBuffer, "object_data_buffer");
		m_ObjectDataBuffer.Write(std::as_bytes(std::span{ objectData.data(), objectData.size() }), 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_ObjectDataBuffer.GetNativeHandle());

		scene.materialManager.Sync();
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, scene.materialManager.GetNativeHandle());

		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, reinterpret_cast<const void*>(m_CommandBuffer.OffsetBytes()), commandCount, 0);

		m_CommandBuffer.Advance();
		m_CameraBuffer.Advance();
		m_ObjectDataBuffer.Advance();
		scene.materialManager.Advance();
	}

}
