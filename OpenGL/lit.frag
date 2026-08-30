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
    float alphaCutoff;
    float alpha;
    bool isMasked;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;

    float constant;
    float linear;
    float quadratic;
};

struct DirLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

#define MAX_POINT_LIGHTS 16
#define MAX_DIR_LIGHTS 4

uniform Material material;

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;

uniform DirLight dirLights[MAX_DIR_LIGHTS];
uniform int numDirLights;

out vec4 FragColor;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

void main()
{
    vec4 texColor = texture(material.diffuse, TexCoords);

    if (material.isMasked && (texColor.a < material.alphaCutoff)) {
        discard;
    }
        
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = ambientStrength * ambientColor * material.color * texColor.rgb;

    for (int i = 0; i < numDirLights && i < MAX_DIR_LIGHTS; i++) {
        result += CalcDirLight(dirLights[i], norm, viewDir);
    }

    if (pointLightsEnabled) {
        for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
            result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
        }
    }

    vec3 emissiveColor = material.color * texColor.rgb;
    result += emissiveColor * material.emissive;

    FragColor = vec4(result, texColor.a * material.alpha);
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
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 radiance = light.color * light.intensity * attenuation;

    vec3 diffuseTex  = vec3(texture(material.diffuse, TexCoords));
    vec3 specularTex = vec3(texture(material.specular, TexCoords));

    vec3 diffuse  = radiance * diff * diffuseTex * material.color;
    vec3 specular = radiance * spec * specularTex;

    return diffuse + specular;
}