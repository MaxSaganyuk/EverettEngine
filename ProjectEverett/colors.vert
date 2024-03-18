#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 inv;

void main()
{
	FragPos = vec3(model * vec4(aPos, 1.0f));
	Normal = mat3(transpose(inv)) * aNormal;
	//Normal = aNormal;

	gl_Position = proj * view * vec4(FragPos, 1.0f);
}