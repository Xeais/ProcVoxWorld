#version 430 core

in vec2 vTexCoord;
flat in uint vTile;

out vec4 outColor;

uniform sampler2DArray textureSampler;
uniform float blockLight;

void main()
{ 
  vec4 color = texture(textureSampler, vec3(vTexCoord, vTile));
  if(color.a < 0.5)
    discard;

  outColor = vec4(color.rgb * blockLight, color.a);
}