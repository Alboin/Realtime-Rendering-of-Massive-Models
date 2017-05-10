#version 330 core

in vec3 vertexColor;
in vec3 vertexNormal;
in vec3 fragPos;

out vec4 color;

uniform vec3 lightPos;

void main()
{
	vec3 lightDir = normalize(lightPos - fragPos);  
	float diff = max(dot(vertexNormal, lightDir), 0.0);
	vec3 diffuse = diff * vec3(1.0f, 1.0f, 1.0f);

	float ambient = 0.2f;

	vec3 result = (diffuse + ambient) * vertexColor;
	color = vec4(result, 1.0f);
}