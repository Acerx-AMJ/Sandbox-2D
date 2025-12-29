#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
in float timeThing;

uniform sampler2D texture0;

out vec4 finalColor;

void main() {
   vec2 uv = fragTexCoord;
   uv.x += timeThing * -0.25;

   vec4 texel = texture(texture0, uv);
   finalColor = texel * fragColor;
}