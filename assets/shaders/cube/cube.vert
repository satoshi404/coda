#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;

//layout(binding = 0) uniform UniformBufferObject
//{
//    mat4 uMVP;
//    mat4 uModel;
//} ubo;

uniform mat4 uMVP;
uniform mat4 uModel;

out vec3 fragPos;
out vec3 fragNormal;
out vec3 fragColor;

void main() {
    gl_Position = uMVP * vec4(inPosition, 1.0);
    fragPos = vec3(uModel * vec4(inPosition, 1.0));
    fragNormal = mat3(uModel) * inNormal;
    fragColor = inColor;
}