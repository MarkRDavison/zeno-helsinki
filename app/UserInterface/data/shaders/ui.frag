#version 450

layout(binding = 1) uniform sampler2D textures[64];

layout(location = 0) in vec3 fragColor; 
layout(location = 1) in vec2 texUv; 
layout(location = 2) in flat uint textureIndex;
layout(location = 3) in flat uint renderType;

layout(location = 0) out vec4 outFragColor;

void main()
{
	vec4 texCol = texture(textures[textureIndex], texUv);
	if (renderType == 0)
	{
		outFragColor = texCol * vec4(fragColor, 1.0f);
	}
	else if (renderType == 1)
	{
		// TODO: Proper Text rendering....
		float dist = texCol.r;

		// Edge depends on how my sdf is normalize
		float edge = 0.15;
		float width = fwidth(dist);

		float alpha = smoothstep(edge - width, edge + width, dist);

		outFragColor = vec4(fragColor * alpha, alpha);
	}
	else 
	{
		outFragColor = vec4(1.0, 1.0, 0.0, 1.0);
	}
}
