#version 330
in vec2 Color;


uniform sampler2D Tex1;

out vec4 FragColor;

void main()
{
	vec4 texColor = texture(Tex1, Color);

	FragColor = texColor;
}