//

#include <common.sp>

layout (location=0) in vec3 vColor;
layout (location=1) in vec3 vNormal;
layout (location=2) in vec3 vFragPos;

layout (location=0) out vec4 out_FragColor;

layout (constant_id = 0) const bool isWireframe = false;

void main() {
	// Object base color
	vec3 objectColor = vec3(pc.color);

	// Ambient
	vec3 ambientLight = vec3(1.0f, 1.0f, 1.0f);
	float ambientStrength = 1.0f;
	vec3 ambient = ambientStrength * ambientLight;
	
	// Result
	vec4 finalColor = vec4(objectColor * ambient, 1.0f);
	
	out_FragColor = isWireframe ? vec4(0.0f, 0.0f, 0.0f, 1.0f) : finalColor;
};