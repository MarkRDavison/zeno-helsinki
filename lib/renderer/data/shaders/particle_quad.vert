#version 450

struct Particle
{
	vec3 position;
	float life;
	vec3 velocity;
	float pad;
};

layout(binding = 0) uniform CameraBuffer
{
	mat4 view;
	mat4 proj;
} ubo[4];

layout(std430, binding = 1) readonly buffer ParticleSSBO
{
	Particle particles[];
};

layout(std140, binding = 2) uniform EmitterUBO
{
	vec4 origin;
	vec4 gravity;
	uvec4 counts;
	uvec4 limits;
} emitter;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec4 fragColor;

const vec2 corners[4] = vec2[](
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(0.0, 1.0),
	vec2(1.0, 1.0)
);

int cornerForVertex(uint v)
{
	const int map[6] = int[6](0, 1, 2, 2, 1, 3);
	return map[v];
}

void main()
{
	uint particleId = uint(gl_VertexIndex) / 6u;
	uint maxP = emitter.limits.y;
	if (particleId >= maxP)
	{
		gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
		fragTexCoord = vec2(0.0);
		fragColor = vec4(0.0);
		return;
	}

	Particle p = particles[emitter.counts.y * maxP + particleId];
	if (p.life <= 0.0)
	{
		gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
		fragTexCoord = vec2(0.0);
		fragColor = vec4(0.0);
		return;
	}

	uint cam = min(emitter.counts.w, 3u);
	mat4 view = ubo[cam].view;
	vec3 camRight = vec3(view[0][0], view[1][0], view[2][0]);
	vec3 camUp = vec3(view[0][1], view[1][1], view[2][1]);

	const float quadSize = 0.15;
	int ci = cornerForVertex(uint(gl_VertexIndex) % 6u);
	vec2 local = corners[ci] - vec2(0.5);
	vec3 world = p.position + camRight * local.x * quadSize + camUp * local.y * quadSize;

	gl_Position = ubo[cam].proj * view * vec4(world, 1.0);
	fragTexCoord = corners[ci];
	float a = clamp(p.life, 0.0, 1.0);
	fragColor = vec4(1.0, 0.85, 0.35, a);
}
