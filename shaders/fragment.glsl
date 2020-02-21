#version 330 core

struct Material {
	float someFloat;
	sampler2D diffuse;
	sampler2D specular;
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

vec3 CalcSpotLight(SpotLight light, vec3 fragPos, vec3 viewDir, SampledMaterial sMaterial)
{

	float theta = dot(viewDir, normalize(-light.direction));
	float epsilon = light.innerCutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	
	vec3 ambient = light.ambient * intensity * vec3(sMaterial.diffuse);
	vec3 diffuse  = light.diffuse * intensity  * vec3(sMaterial.diffuse);
	vec3 specular = light.specular * intensity  * vec3(sMaterial.specular);

	return ambient + diffuse + specular;
}
/////////////////////////////////////////////////////////////////////////////////

uniform DirLight dirLight;

#define NR_POINT_LIGHTS 10
uniform PointLight pointLights[NR_POINT_LIGHTS];

uniform SpotLight spotLight;

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;

void main()
{
    vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);

	SampledMaterial sMaterial;
	sMaterial.diffuse = texture(material.diffuse, TexCoords);
	sMaterial.specular = texture(material.specular, TexCoords);
	sMaterial.shininess = material.shininess;
	/*vec3 result = CalcDirLight(dirLight, norm, viewDir, sMaterial);
	for(int i = 0; i < NR_POINT_LIGHTS; i++)
		result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, sMaterial);
	result += CalcSpotLight(spotLight, FragPos, viewDir, sMaterial);*/
	
	//FragColor = vec4(result, 1.0);
	FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}