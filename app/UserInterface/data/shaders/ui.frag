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

		float c = texCol.r * 4.0;
		c = clamp(c, 0.0, 1.0);

		float g = fwidth(c);

		float edge = abs(c - 0.5);
		float line = 1.0 - smoothstep(g * 1.5, g * 3.0, edge);

		float thickness = 0.02;
		float threshold = 0.65;

		float fill = smoothstep(threshold - thickness, threshold + thickness, c);

		vec3 outlineColor = vec3(0.0);
		vec3 col = mix(outlineColor, fragColor, fill);

		float alpha = max(fill, line);

		outFragColor = vec4(col * alpha, alpha);
	}
	else 
	{
		outFragColor = vec4(1.0, 1.0, 0.0, 1.0);
	}
}
