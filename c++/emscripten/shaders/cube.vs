attribute vec3 aPos;
attribute vec3 aNormal;
attribute vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

varying vec4 vertexPos;
varying vec2 TexCoords;
varying vec3 FragPos;
varying vec3 Normal;

mat4 inverse(in mat4 c) {
  return c; // TODO
}

mat4 transpose(in mat4 c) {
  return c; // TODO
}

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  FragPos = vec3(model * vec4(aPos, 1.0));
  Normal = mat3(transpose(inverse(model))) * aNormal;
  TexCoords = aTexCoords;
}