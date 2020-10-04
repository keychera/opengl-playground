#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;

struct Material {
  sampler2D diffuse;
};
uniform Material material;

void main() {
  vec4 texel = texture(material.diffuse, TexCoords);
  if (texel.a < 0.01)
    discard;
  FragColor = texel;
}