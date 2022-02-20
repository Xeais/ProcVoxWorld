#version 430 core

in vec2 vTexCoord;
out vec4 outColor;

uniform sampler2DArray texSampler;

void main()
{
  outColor = texture(texSampler, vec3(vTexCoord, 0));
}