#version 330 core
out vec4 FragColor;

in vec4 vertexColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float mixVal;
uniform float zoom;

void main()
{
    FragColor = mix(texture(texture1, TexCoord), texture(texture2, vec2(-1.0f * zoom * TexCoord.x, zoom * TexCoord.y)), mixVal);
}