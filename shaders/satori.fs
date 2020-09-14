#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 viewPos;

struct Light {
  vec3 position;

  vec3 ambient;
  vec3 diffuse;

  float constant;
  float linear;
  float quadratic;
};
uniform Light light;

struct Material {
  sampler2D diffuse;
};
uniform Material material;

void main() {
  vec3 lightDir = normalize(light.position - FragPos);

  // ambient
  vec4 ambient = vec4(light.ambient, 0.0) * texture(material.diffuse, TexCoords);

  // diffuse
  vec3 norm = normalize(Normal);
  float diff = max(dot(norm, lightDir), 0.0);
  vec4 diffuse =
      vec4(light.diffuse, 1.0) * diff * texture(material.diffuse, TexCoords);

  vec4 result = ambient + diffuse;
  FragColor = result;
}