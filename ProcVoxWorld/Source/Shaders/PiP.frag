#version 430 core

in vec2 vTexCoord;

out vec4 outColor;

uniform sampler2D uTexture;

void main()
{   
  float texColor = texture(uTexture, vTexCoord).r;
  outColor = vec4(vec3(texColor), 1.0);
}