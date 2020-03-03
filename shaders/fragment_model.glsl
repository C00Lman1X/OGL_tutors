#version 330 core
struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	float shininess;
};

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform Material material;

void main()
{	
	vec4 texColor = texture(material.texture_diffuse1, TexCoords);
	FragColor = texColor;
    //FragColor = vec4(0.5216, 0.1176, 0.1176, 1.0);
}