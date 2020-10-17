precision mediump float;

varying vec3 FragPos;
varying vec2 TexCoords;

struct Material {
  sampler2D diffuse;
};
uniform Material material;

void main() {
  vec4 texel = texture2D(material.diffuse, TexCoords);
  if (texel.a < 0.01)
    discard;
  gl_FragColor = texel;
}