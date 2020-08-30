#version 330 core
out vec4 FragColor;
uniform float ourGreenColor;

void main()
{
    FragColor = vec4(0.0f, ourGreenColor, 0.0f, 1.0f);
};