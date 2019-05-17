#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

out vec2 texCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main(){
	// gl_Position = projection * view * model * vec4(aPos, 1.0f);
	FragPos = vec3(model * vec4(aPos, 1.0f));
	Normal = mat3(transpose(inverse(model))) * aNormal;

	vec4 temp = projection * view * model * vec4(aPos, 1.0f);
	temp.x = temp.x / temp.z;
	temp.y = temp.y / temp.z;
	temp.x = - (temp.x - 500)/ 500.0;
	temp.y = - (temp.y - 281)/ 281.0;
	temp.z = - (1 + temp.z / 10000.0);
	temp.w = 1;
	gl_Position = temp;
	texCoord = vec2(aTexCoord.x, aTexCoord.y);
}