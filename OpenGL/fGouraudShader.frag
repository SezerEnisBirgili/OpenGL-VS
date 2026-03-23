#version 330 core

in vec3 LightingColor;
in vec2 TexCoord;

uniform sampler2D texture1;

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(texture1, TexCoord);
    FragColor = vec4(LightingColor * texColor.rgb, 1.0);
}