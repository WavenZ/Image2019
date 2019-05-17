#version 330 core

in vec2 texCoord;
in vec3 Normal;
in vec3 FragPos;
out vec4 outColor;

uniform sampler2D ourTexture;
uniform vec3 lightPos;
void main(){
	
	vec3 ambient = vec3(0.8f, 0.8f, 0.8f);
	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
	vec3 viewPos = vec3(3.0f, 3.0f, 10.0f);

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * lightColor;

	float specularStrength = 0.5f;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 128);
	vec3 specular = specularStrength * spec * lightColor;
	outColor = vec4(ambient + diffuse + specular, 1.0f) * texture(ourTexture, texCoord);

	//vec4 texColor = texture(ourTexture, texCoord);
    //if(texColor.a < 0.2)
    //    discard;
    //outColor = texColor;
}