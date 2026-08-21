#version 330 core
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 viewPos;

uniform float ambientStrength;
uniform vec3 ambientColor;
uniform bool pointLightsEnabled;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    vec3 color;
    float shininess;
    float emissive;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;

    float constant;
    float linear;
    float quadratic;
};

#define MAX_POINT_LIGHTS 16

struct DirLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

uniform Material material;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;

uniform DirLight dirLight;
uniform bool hasDirLight;

out vec4 FragColor;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = ambientStrength * ambientColor * material.color * vec3(texture(material.diffuse, TexCoords));

    if (hasDirLight)
        result += CalcDirLight(dirLight, norm, viewDir);

    if (pointLightsEnabled) {
        for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++)
            result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }

    result = mix(result, material.color, material.emissive);

    FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{

    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

    vec3 radiance = light.color * light.intensity;

    vec3 diffuseTex  = vec3(texture(material.diffuse, TexCoords));
    vec3 specularTex = vec3(texture(material.specular, TexCoords));

    vec3 diffuse  = radiance * diff * diffuseTex * material.color;
    vec3 specular = radiance * spec * specularTex;

    return diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);


    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                                light.quadratic * (distance * distance));

    vec3 radiance = light.color * light.intensity * attenuation;

    vec3 diffuseTex  = vec3(texture(material.diffuse, TexCoords));
    vec3 specularTex = vec3(texture(material.specular, TexCoords));

    vec3 diffuse  = radiance * diff * diffuseTex * material.color;
    vec3 specular = radiance * spec * specularTex;

    return diffuse + specular;
}
