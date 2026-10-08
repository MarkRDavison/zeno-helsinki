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

layout(location = 0) out vec4 fragColor;

void main()
{
	uint id = uint(gl_VertexIndex);
	uint maxP = emitter.limits.y;
	if (id >= maxP)
	{
		gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
		gl_PointSize = 1.0;
		fragColor = vec4(0.0);
		return;
	}

	Particle p = particles[emitter.counts.y * maxP + id];
	if (p.life <= 0.0)
	{
		gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
		gl_PointSize = 1.0;
		fragColor = vec4(0.0);
		return;
	}

	uint cam = min(emitter.counts.w, 3u);
	gl_Position = ubo[cam].proj * ubo[cam].view * vec4(p.position, 1.0);
	gl_PointSize = 10.0;
	float a = clamp(p.life, 0.0, 1.0);
	fragColor = vec4(1.0, 0.85, 0.35, a);
}
