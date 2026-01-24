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

layout(binding = 0, std430) readonly buffer vertices
{
	VertexData data[];
};

layout(binding = 1, std430) readonly buffer textures_buffer
{
	sampler2D textures[];
};

layout(binding = 2, std430) readonly buffer lights
{
	float ambientColor[3];
	float pointLightPos[3];
	float pointLightColor[3];
	float pointLightAttenuation[3];
	float pointLightSpecularPower;
};

layout(binding = 3, std430) readonly buffer camera
{
	mat4 view;
	mat4 projection;
	float cameraPosition[3];
	float pad;
};

layout(location = 0) uniform uint albedo_tex_index;
layout(location = 1) uniform uint normal_tex_index;
layout(location = 2) uniform uint position_tex_index;
layout(location = 3) uniform uint specular_tex_index;

layout(location = 0) in vec4 in_frag_position;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

void main()
{
	vec3 albedo = texture(textures[albedo_tex_index], in_uv).rgb;
	vec3 normal = texture(textures[normal_tex_index], in_uv).xyz;
	vec3 fragPos = texture(textures[position_tex_index], in_uv).xyz;
	float specular = texture(textures[specular_tex_index], in_uv).r;

	vec3 pointPos = vec3(pointLightPos[0], pointLightPos[1], pointLightPos[2]);
	vec3 pointColor = vec3(pointLightColor[0], pointLightColor[1], pointLightColor[2]);
	vec3 pointAttenuation = vec3(pointLightAttenuation[0], pointLightAttenuation[1], pointLightAttenuation[2]);

	vec3 ambient = vec3(ambientColor[0], ambientColor[1], ambientColor[2]);
	vec3 lightDir = normalize(pointPos - fragPos);
	float diffuse = max(dot(normal, lightDir), 0.0);

	vec3 cameraPos = vec3(cameraPosition[0], cameraPosition[1], cameraPosition[2]);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normalize(cameraPos - fragPos), reflectDir), 0.0), pointLightSpecularPower) * specular;

	float distance = length(pointPos - fragPos);
	float att = 1.0 / (pointAttenuation.x + (pointAttenuation.y * distance) + (pointAttenuation.z * (distance * distance)));

	vec3 lighting = (ambient * albedo) + ((diffuse + spec) * att) * pointColor * albedo;

	out_color = vec4(lighting, 1.0);
}