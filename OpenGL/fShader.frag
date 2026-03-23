#version 330 core
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;

uniform float mixFactor;

uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;
uniform int shininess;

uniform sampler2D texture1;


out vec4 FragColor;
  
void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos); 

    vec3 ambient = ambientStrength * lightColor;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * diff * lightColor;

    vec3 viewDir = normalize(-FragPos);
    vec3 reflectDir = reflect(-lightDir, norm); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    vec4 texColor = texture(texture1, TexCoord);

    vec3 result = (ambient + diffuse + specular) * objectColor * texColor.rgb;
    FragColor = vec4(result, texColor.a);
}   