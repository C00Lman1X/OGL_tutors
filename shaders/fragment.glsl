

#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float mixValue;

void main()
{
	vec2 coord1 = TexCoord;
	vec2 coord2 = vec2(TexCoord.x, TexCoord.y);
	FragColor = mix(texture(texture1, coord1), texture(texture2, coord2), mixValue);
	//FragColor = texture(texture2, TexCoord);
}

