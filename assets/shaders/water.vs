#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

uniform mat4 mvp;
uniform float time;

out vec2 fragTexCoord;
out vec4 fragColor;

void main() {
   vec3 position = vertexPosition;
   bool isTop = vertexColor.r > 0.5;
   bool isBottom = vertexColor.g > 0.5;

   if (!(isTop && isBottom) && ((isTop && !isBottom && vertexTexCoord.y == 1.0) || (!isTop && isBottom && vertexTexCoord.y == 0.0) || (!isTop && !isBottom))) {
      position.y += sin(time * 2.0 + position.x) * 0.15;
   }

   gl_Position = mvp * vec4(position, 1.0);
   fragTexCoord = vertexTexCoord + vec2(time * -0.25, 0.0);
   fragColor = vec4(1.0, 1.0, 1.0, vertexColor.a);
}
