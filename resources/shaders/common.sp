//

layout(std430, buffer_reference) readonly buffer UniformData {
	mat4 model;
	mat4 view;
	mat4 proj;
	vec4 color;
	vec4 cameraPosition;
	uint outputTexId;
};

layout(push_constant) uniform PushConstants {
	UniformData pc;
};