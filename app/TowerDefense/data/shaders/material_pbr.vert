#version 450

layout(push_constant) uniform PushConstants {
    mat4 model;
    int materialIndex;
    int cameraIndex;
    int skipShadow;
    int pad;
} pc;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo[4];

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out flat int fragMaterialIndex;
layout(location = 3) out vec3 fragNormal;
layout(location = 4) out vec3 fragWorldPos;
layout(location = 5) out flat int fragCameraIndex;
layout(location = 6) out flat int fragSkipShadow;

void main() {
    gl_Position = ubo[pc.cameraIndex].proj * ubo[pc.cameraIndex].view * pc.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragMaterialIndex = pc.materialIndex;
    fragCameraIndex = pc.cameraIndex;
    fragSkipShadow = pc.skipShadow;

    mat3 model3 = mat3(pc.model);
    fragNormal = transpose(inverse(model3)) * inNormal;
    fragWorldPos = (pc.model * vec4(inPosition, 1.0)).xyz;
}
