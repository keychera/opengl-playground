precision mediump float;

varying vec3 FragPos;
varying vec2 TexCoords;

struct Light {
  vec3 position;

  float ambient;
  float intensity;
  sampler2D colorRamp;
};
uniform Light light;

struct Material {
  sampler2D diffuse;
  sampler2D normal;
};

uniform Material material;

void main() {
  vec4 texel = texture2D(material.diffuse, TexCoords);
  if (texel.a < 0.01)
    discard;

  // normal intensity
  vec3 lightDir = normalize(vec3(light.position.xy, 0));
  vec3 normal = texture2D(material.normal, TexCoords).rgb;
  vec3 modNormal = normalize(normal * 2.0 - 1.0);
  float dotVal = dot(modNormal, lightDir);
  dotVal = pow(cos((dotVal * 0.7853) - 0.8), 1.4);
  float intensity = max(dotVal, 0.0);
  intensity *= light.intensity;

  // toon
  vec4 lightFactor = texture2D(light.colorRamp, vec2(intensity, 0.5));

  vec4 result = lightFactor * texel * vec4(vec3(light.ambient), 1.0);
  gl_FragColor = result;
}