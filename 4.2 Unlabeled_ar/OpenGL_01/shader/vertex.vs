#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main(){
	// gl_Position = projection * view * model * vec4(aPos, 1.0f);
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