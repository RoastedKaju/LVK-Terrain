//

#include <common.sp>

layout (location=0) in vec2 vUV;

layout (location=0) out vec4 out_FragColor;

void main() {
    vec4 color = textureBindless2D(pc.outputTexId, 0, vUV);

    // Remaps UV from [0, 1] to [-1, 1]
    vec2 centered = vUV * 2.0 - 1.0;

    // Chromatic aberration
    vec2 offset = centered * pc.chromaticAberrationStrength;
    float r = textureBindless2D(pc.outputTexId, 0, vUV + offset).r;
    float g = textureBindless2D(pc.outputTexId, 0, vUV).g;
    float b = textureBindless2D(pc.outputTexId, 0, vUV - offset).b;
    color = vec4(r, g, b, 1.0);

    float exposure = pc.exposure; // tweak this
    color.rgb *= exposure;
    color.rgb = color.rgb / (color.rgb + vec3(1.0)); // Reinhard
    color.rgb = pow(color.rgb, vec3(1.0 / 2.2));     // Gamma

    out_FragColor = vec4(color.rgb, color.a);
}