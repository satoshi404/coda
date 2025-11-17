#version 450

in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragColor;

// /layout(binding = 0) uniform UniformBufferObject 
// /{
// mat4 uMVP;
// mat4 uModel;
// /} ubo;

uniform sampler2D texSampler;

out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}