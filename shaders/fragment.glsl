#version 330 core

struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_diffuse2;
	sampler2D texture_diffuse3;
	sampler2D texture_diffuse4;
	sampler2D texture_diffuse5;
	sampler2D texture_specular1;
	sampler2D texture_specular2;
	sampler2D texture_specular3;
	sampler2D texture_specular4;
	sampler2D texture_specular5;
	float shininess;
};
struct SampledMaterial {
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

///////////////////// light calculation /////////////////////////////////////////
struct DirLight {
	vec3 direction;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight {
	vec3 direction;
	vec3 position;
	float innerCutOff;
	float outerCutOff;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, SampledMaterial sMaterial)
{
	vec3 lightDir = normalize(-light.direction);

	float diff = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), sMaterial.shininess);

	vec3 ambient = light.ambient * vec3(sMaterial.diffuse);
	vec3 diffuse = light.diffuse * diff * vec3(sMaterial.diffuse);
	vec3 specular = light.specular * spec * vec3(sMaterial.specular);

	return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, SampledMaterial sMaterial)
{
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), sMaterial.shininess);

	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

	vec3 ambient = light.ambient * vec3(sMaterial.diffuse);
	vec3 diffuse = light.diffuse * diff * vec3(sMaterial.diffuse);
	vec3 specular = light.specular * spec * vec3(sMaterial.specular);
	return (ambient + diffuse + specular) * attenuation;
}

vec3 CalcSpotLight(SpotLight light, vec3 fragPos, SampledMaterial sMaterial)
{
	vec3 fragToLightDir = normalize(light.position - fragPos);

	float theta = dot(fragToLightDir, normalize(-light.direction));
	float epsilon = light.innerCutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	
	vec3 ambient = light.ambient * intensity * vec3(sMaterial.diffuse);
	vec3 diffuse  = light.diffuse * intensity  * vec3(sMaterial.diffuse);
	vec3 specular = light.specular * intensity  * vec3(sMaterial.specular);

	return ambient + diffuse + specular;
}
/////////////////////////////////////////////////////////////////////////////////

#define NR_DIR_LIGHTS 10
uniform DirLight dirLights[NR_DIR_LIGHTS];
uniform int dirLightsCount;

#define NR_POINT_LIGHTS 10
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform int pointLightsCount;

#define NR_SPOT_LIGHTS 10
uniform SpotLight spotLights[NR_SPOT_LIGHTS];
uniform int spotLightsCount;

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform bool isSolidColor;
uniform vec4 color;
uniform bool opaque;

void main()
{
    vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);

	SampledMaterial sMaterial;
	sMaterial.diffuse = texture(material.texture_diffuse1, TexCoords);
	sMaterial.specular = texture(material.texture_specular1, TexCoords);
	sMaterial.shininess = material.shininess;
	
	float alpha = sMaterial.diffuse.a;

	if (isSolidColor)
	{
		sMaterial.diffuse = color;
		sMaterial.specular = color;
		alpha = color.a;
	}

	if (opaque)
		alpha = 1.0;

	vec3 result = vec3(0.0, 0.0, 0.0);
	for(int i = 0; i < NR_DIR_LIGHTS; i++)
	{
		if (i >= dirLightsCount)
			break;
		result += CalcDirLight(dirLights[i], norm, viewDir, sMaterial);
	}
	for(int i = 0; i < NR_POINT_LIGHTS; i++)
	{
		if (i >= pointLightsCount)
			break;
		result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, sMaterial);
	}
	for(int i = 0; i < NR_SPOT_LIGHTS; i++)
	{
		if (i >= spotLightsCount)
			break;
		result += CalcSpotLight(spotLights[i], FragPos, sMaterial);
	}
	
	FragColor = vec4(result, alpha);
	//FragColor = vec4(sMaterial.diffuse);
}