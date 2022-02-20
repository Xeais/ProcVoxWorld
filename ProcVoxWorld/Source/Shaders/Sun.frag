#version 430 core

in vec2 vTexCoord;

out vec4 outColor;

uniform sampler2D textureSampler;

void main()
{ 
  outColor = texture(textureSampler, vTexCoord);
}