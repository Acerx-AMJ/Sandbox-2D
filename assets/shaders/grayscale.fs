#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;

out vec4 finalColor;

void main() {
   vec2 uv = fragTexCoord;
   vec4 color = texture(texture0, uv);

   if (uv.x > fragColor.a) {
      float linearY = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
      finalColor = vec4(linearY / 3.0, linearY / 3.0, linearY / 3.0, color.a); // Make the color 3x darker than it should be
   } else {
      finalColor = vec4(color.rgb, color.a);
   }
}
