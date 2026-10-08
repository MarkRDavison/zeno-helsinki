#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 0) out vec4 outColor;

void main()
{
	vec2 c = gl_PointCoord * 2.0 - 1.0;
	if (dot(c, c) > 1.0)
	{
		discard;
	}

	outColor = fragColor;
}
