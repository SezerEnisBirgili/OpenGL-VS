#version 330 core
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform vec3 lightPos;
uniform vec3 viewPos;

struct Material {
    sampler2D diffuse;
    sampler2D emissive;
    sampler2D specular;
    float shininess;
};
uniform Material material;

struct Light {
    vec3 position;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;

uniform float time;

out vec4 FragColor;
  
void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos); 

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));

    vec3 emissiveColor = vec3(
        sin(time * 2.0) * 0.5 + 0.5,
        sin(time * 1.3 + 2.0) * 0.5 + 0.5,
        sin(time * 0.7 + 4.0) * 0.5 + 0.5
    );
    vec3 emissive = emissiveColor * vec3(texture(material.emissive, TexCoord));

    FragColor = vec4(ambient + diffuse + specular + emissive, 1.0); 
}   