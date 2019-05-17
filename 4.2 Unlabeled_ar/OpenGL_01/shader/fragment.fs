#version 330 core

in vec2 texCoord;
out vec4 outColor;

uniform sampler2D ourTexture;
void main(){
	 outColor = texture(ourTexture, texCoord);
	//vec4 texColor = texture(ourTexture, texCoord);
    //if(texColor.a < 0.2)
    //    discard;
    //outColor = texColor;
}