precision mediump float;

varying vec3 FragPos;
varying vec2 TexCoords;

struct Material {
  sampler2D diffuse;
};
uniform Material material;

struct Light {
  float ambient;
};
uniform Light light;

void main() {
  vec4 texel = texture2D(material.diffuse, TexCoords);
  if (texel.a < 0.01)
    discard;
  vec4 result = texel * vec4(vec3(light.ambient), 1.0);
  gl_FragColor = result;
}