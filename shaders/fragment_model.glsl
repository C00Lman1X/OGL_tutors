#version 330 core
struct Material {
	sampler2D texture_diffuse;
	sampler2D texture_specular;
	float shininess;
};

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform Material material;

void main()
{	
	FragColor = vec4(vec3(texture(material.texture_diffuse, TexCoords)), 1.0);
    //FragColor = vec4(0.5, 0.5, 0.5, 1.0);
}