#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec4 vertexColor;
out vec4 vertexPos;
out vec2 TexCoord;

uniform float offset;
uniform mat4 transform;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
  vertexColor = vec4(aColor, 1.0f);
  gl_Position = projection * view * model * transform * vec4(aPos.x + offset, aPos.y + offset, aPos.z, 1.0f);
  vertexPos = gl_Position;
  TexCoord = aTexCoord;
};