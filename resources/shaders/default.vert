//

#include <common.sp>

layout (location=0) in vec3 inPos;
layout (location=1) in vec3 inNormal;
layout (location=2) in vec2 inUV;

layout (location=0) out vec3 vColor;
layout (location=1) out vec3 vNormal;
layout (location=2) out vec3 vFragPos;

layout (constant_id = 0) const bool isWireframe = false;

void main()
{
	gl_Position = pc.proj * pc.view * pc.model * vec4(inPos, 1.0f);

	vColor = isWireframe ? vec3(0.0f) : inPos.xyz;
	vNormal = mat3(transpose(inverse(pc.model))) * inNormal; 
	vFragPos = vec3(pc.model * vec4(inPos, 1.0f));
}