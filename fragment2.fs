#version 330 core
out vec4 FragColor;
in vec4 vertexPos;
uniform float ourGreenColor;

void main()
{
    FragColor = vec4(vertexPos.r, vertexPos.g + ourGreenColor, vertexPos.b, 1.0f);
};