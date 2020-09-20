#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;

struct Light {
  vec3 position;
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
  if (texel.a < 0.01)
    discard;

  // normal intensity
  vec3 lightDir = normalize(vec3(light.position.xy, 0));
  vec3 normal = texture(material.normal, TexCoords).rgb;
  vec3 modNormal = -1.0 * normalize(normal * 2.0 - 1.0);
  float intensity = max(dot(modNormal, lightDir), 0.0);

  // toon
  vec4 lightFactor = texture(light.colorRamp, intensity);

  vec4 result = lightFactor * texel;
  FragColor = result;
}