#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;

uniform mat4 mvp;
uniform float time;
uniform float amplitude;
uniform float speed;

out vec2 fragTexCoord;

void main() {
   vec3 position = vertexPosition;
   if (vertexTexCoord.y == 0.0) {
      position.y += sin(time * speed + position.x * 0.01) * amplitude;
   }
   gl_Position = mvp * vec4(position, 1.0);
   fragTexCoord = vertexTexCoord;
}
