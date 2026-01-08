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