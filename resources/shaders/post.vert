//

#include <common.sp>

layout (location=0) out vec2 vUV;

// Generates a fullscreen triangle from gl_VertexIndex (0,1,2)
void main() {
    vUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(vUV * 2.0 - 1.0, 0.0, 1.0);
    vUV.y = 1.0 - vUV.y; // flip AFTER computing position, before passing to frag
}