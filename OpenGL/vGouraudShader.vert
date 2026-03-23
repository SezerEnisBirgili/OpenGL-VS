#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightColor;
uniform vec3 lightPos;

uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;
uniform int shininess;

out vec3 LightingColor;
out vec2 TexCoord;


void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    vec3 FragPos = vec3(model * vec4(aPos, 1.0));

    vec3 norm =  normalize(mat3(transpose(inverse(view * model))) * aNormal);
    vec3 lightDir = normalize(lightPos - FragPos); 

    vec3 ambient = ambientStrength * lightColor;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * diff * lightColor;

    vec3 viewDir = normalize(-FragPos);
    vec3 reflectDir = reflect(-lightDir, norm); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    LightingColor = ambient + diffuse + specular;
    TexCoord = aTexCoord;

}