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
		float tangent[3];
		float bitangent[3];
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
		float cameraPosition[3];
		float pad;
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
		float pointLightSpecularPower;
	};

	vec3 get_position(uint index)
	{
		return vec3(data[index].position[0], data[index].position[1], data[index].position[2]);
	}

	vec3 get_normal(uint index)
	{
		return vec3(data[index].normal[0], data[index].normal[1], data[index].normal[2]);
	}

	vec3 get_tangent(uint index)
	{
		return vec3(data[index].tangent[0], data[index].tangent[1], data[index].tangent[2]);
	}

	vec3 get_bitangent(uint index)
	{
		return vec3(data[index].bitangent[0], data[index].bitangent[1], data[index].bitangent[2]);
	}

	vec2 get_uv(uint index)
	{
		return vec2(data[index].uv[0], data[index].uv[1]);
	}

	layout(location = 0) out flat uint out_material_index;
	layout(location = 1) out vec2 out_uv;
	layout(location = 2) out vec4 out_frag_position;
	layout(location = 3) out mat3 out_tbn;

	void main()
	{
		mat3 normalMat = transpose(inverse(mat3(objectData[gl_DrawID].model)));

		out_frag_position = objectData[gl_DrawID].model * vec4(get_position(gl_VertexID), 1.0);
		gl_Position = projection * view * out_frag_position;
		out_material_index = objectData[gl_DrawID].material_index;
		out_uv = get_uv(gl_VertexID);

		vec3 t = normalize(vec3(objectData[gl_DrawID].model * vec4(get_tangent(gl_VertexID), 0.0)));
		vec3 b = normalize(vec3(objectData[gl_DrawID].model * vec4(get_bitangent(gl_VertexID), 0.0)));
		vec3 n = normalize(vec3(objectData[gl_DrawID].model * vec4(get_normal(gl_VertexID), 0.0)));
		out_tbn = mat3(t, b, n);
	}
	)"sv;

	constexpr auto sampleFragmentShader = R"(
	#version 460 core
	#extension GL_ARB_bindless_texture : require
	
	struct VertexData
	{
		float position[3];
		float normal[3];
		float tangent[3];
		float bitangent[3];
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
		float cameraPosition[3];
		float pad;
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
		float pointLightSpecularPower;
	};

	layout(location = 0, bindless_sampler) uniform sampler2D albedo_tex;
	layout(location = 1, bindless_sampler) uniform sampler2D normal_tex;
	layout(location = 2, bindless_sampler) uniform sampler2D specular_tex;

	layout(location = 0) in flat uint in_material_index;
	layout(location = 1) in vec2 in_uv;
	layout(location = 2) in vec4 in_frag_position;
	layout(location = 3) in mat3 in_tbn;

	layout(location = 0) out vec4 out_color;

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

		vec3 cameraPos = vec3(cameraPosition[0], cameraPosition[1], cameraPosition[2]);

		vec3 reflectDir = reflect(-lightDir, n);
		float spec = pow(max(dot(normalize(cameraPos - fragPosition), reflectDir), 0.0), pointLightSpecularPower) * texture(specular_tex, in_uv).r;

		return (diff + spec) * att * color;
	}

	void main()
	{
		vec3 n = texture(normal_tex, in_uv).xyz;
		n = (n * 2.0) - 1.0;
		n = normalize(in_tbn * n);

		vec3 albedo = texture(albedo_tex, in_uv).rgb;
		vec3 ambColor = vec3(ambientColor[0], ambientColor[1], ambientColor[2]);
		vec3 pointColor = calc_point(in_frag_position.xyz, n);

		out_color = vec4(albedo * (ambColor + pointColor), 1.0);
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
		glProgramUniformHandleui64ARB(m_Program.GetNativeHandle(), 1, scene.theOneNormal.GetNativeHandle());
		glProgramUniformHandleui64ARB(m_Program.GetNativeHandle(), 2, scene.theOneSpecular.GetNativeHandle());

		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, reinterpret_cast<const void*>(m_CommandBuffer.OffsetBytes()), commandCount, 0);

		m_CommandBuffer.Advance();
		m_CameraBuffer.Advance();
		m_LightBuffer.Advance();
		m_ObjectDataBuffer.Advance();
		scene.materialManager.Advance();
	}

}
