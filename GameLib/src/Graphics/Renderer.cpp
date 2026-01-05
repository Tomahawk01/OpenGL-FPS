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
		float normal[3];
		float uv[2];
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

	layout(binding = 4, std430) readonly buffer lights
	{
		float ambientColor[3];
		float pointLightPos[3];
		float pointLightColor[3];
		float pointLightAttenuation[3];
	};

	vec3 get_position(uint index)
	{
		return vec3(data[index].position[0], data[index].position[1], data[index].position[2]);
	}

	vec3 get_normal(uint index)
	{
		return vec3(data[index].normal[0], data[index].normal[1], data[index].normal[2]);
	}

	vec2 get_uv(uint index)
	{
		return vec2(data[index].uv[0], data[index].uv[1]);
	}

	layout(location = 0) out flat uint out_material_index;
	layout(location = 1) out vec2 out_uv;
	layout(location = 2) out vec3 out_normal;
	layout(location = 3) out vec4 out_frag_position;

	void main()
	{
		out_frag_position = objectData[gl_DrawID].model * vec4(get_position(gl_VertexID), 1.0);
		gl_Position = projection * view * out_frag_position;
		out_material_index = objectData[gl_DrawID].material_index;
		out_uv = get_uv(gl_VertexID);
		out_normal = get_normal(gl_VertexID);
	}
	)"sv;

	constexpr auto sampleFragmentShader = R"(
	#version 460 core
	#extension GL_ARB_bindless_texture : require
	
	struct VertexData
	{
		float position[3];
		float normal[3];
		float uv[2];
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

	layout(binding = 4, std430) readonly buffer lights
	{
		float ambientColor[3];
		float pointLightPos[3];
		float pointLightColor[3];
		float pointLightAttenuation[3];
	};

	vec3 get_color(uint index)
	{
		return vec3(materialData[index].color[0], materialData[index].color[1], materialData[index].color[2]);
	}

	vec3 calc_point(vec3 fragPosition, vec3 n)
	{
		vec3 pos = vec3(pointLightPos[0], pointLightPos[1], pointLightPos[2]);
		vec3 color = vec3(pointLightColor[0], pointLightColor[1], pointLightColor[2]);
		vec3 attenuation = vec3(pointLightAttenuation[0], pointLightAttenuation[1], pointLightAttenuation[2]);

		float distance = length(pos - fragPosition);
		float att = 1.0 / (attenuation.x + (attenuation.y * distance) + (attenuation.z * (distance * distance)));

		vec3 lightDir = normalize(pos - fragPosition);
		float diff = max(dot(n, lightDir), 0.0);

		return diff * att * color;
	}

	layout(location = 0, bindless_sampler) uniform sampler2D tex;

	layout(location = 0) in flat uint in_material_index;
	layout(location = 1) in vec2 in_uv;
	layout(location = 2) in vec3 in_normal;
	layout(location = 3) in vec4 in_frag_position;

	layout(location = 0) out vec4 out_color;

	void main()
	{
		vec3 ambColor = vec3(ambientColor[0], ambientColor[1], ambientColor[2]);
		out_color = vec4(ambColor + calc_point(in_frag_position.xyz, in_normal), 1.0);
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
		, m_LightBuffer{ sizeof(LightData), "light_buffer" }
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

		m_LightBuffer.Write(std::as_bytes(std::span<const LightData, 1>{&scene.lights, 1}), 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_LightBuffer.GetNativeHandle());

		glProgramUniformHandleui64ARB(m_Program.GetNativeHandle(), 0, scene.theOneTexture.GetNativeHandle());

		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, reinterpret_cast<const void*>(m_CommandBuffer.OffsetBytes()), commandCount, 0);

		m_CommandBuffer.Advance();
		m_CameraBuffer.Advance();
		m_LightBuffer.Advance();
		m_ObjectDataBuffer.Advance();
		scene.materialManager.Advance();
	}

}
