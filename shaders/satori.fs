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

  sampler1D colorRamp;
};
uniform Light light;

struct Material {
  sampler2D diffuse;
  sampler2D normal;
};

uniform Material material;

void main() {
  vec4 texel = texture(material.diffuse, TexCoords);
  if (texel.a < 0.1)
    discard;

  vec3 modLightPos = vec3(light.position.xy, 0);
  vec3 lightDir = normalize(modLightPos);

  // ambient
  vec4 ambient = vec4(light.ambient, 0.0) * texel;

  // diffuse + normal
  vec3 normal = texture(material.normal, TexCoords).rgb;
  vec3 norm = -1.0 * normalize(normal * 2.0 - 1.0);
  float diff = max(dot(norm, lightDir), 0.0);

  // toon
  vec4 lightFactor = texture(light.colorRamp, diff);

  vec4 diffuse = vec4(light.diffuse, 1.0) * lightFactor * texel;

  vec4 result = ambient + diffuse;
  FragColor = diffuse;
}