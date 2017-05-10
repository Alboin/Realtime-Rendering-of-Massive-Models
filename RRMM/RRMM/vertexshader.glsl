#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;

uniform mat4 MVP;
uniform mat4 model;

out vec3 vertexColor;
out vec3 vertexNormal;
out vec3 fragPos;

void main()
{
	gl_Position = MVP * vec4(position, 1.0f);

	vertexColor = color;
	vertexNormal = normal;
	fragPos = vec3(model * vec4(position, 1.0f));
}