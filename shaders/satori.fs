#version 330 core
out vec4 FragColor;

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
  sampler2D normal;
  sampler1D colorRamp;
};

uniform Material material;

void main() {
  vec3 modLightPos = vec3(light.position.xy, 0);
  vec3 lightDir = normalize(modLightPos);

  // ambient
  vec4 ambient =
      vec4(light.ambient, 0.0) * texture(material.diffuse, TexCoords);

  // diffuse + normal
  vec3 normal = texture(material.normal, TexCoords).rgb;
  vec3 norm = normalize(normal * 2.0 - 1.0);
  float diff = max(dot(norm, lightDir), 0.0);

  // toon
  vec3 color = texture(material.colorRamp, diff).rgb;

  // vec4 diffuse =
  //     vec4(light.diffuse, 1.0) * diff * texture(material.diffuse, TexCoords);

  float a = texture(material.diffuse, TexCoords).a;
  vec4 result = ambient + vec4(color, a);
  if (result.a < 0.1)
    discard;
  FragColor = result;
}