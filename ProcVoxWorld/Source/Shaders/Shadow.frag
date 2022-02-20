#version 430 core

in vec2 vTexCoord;
flat in uint vTile;

uniform sampler2DArray uBlocksTexture;

void main()
{  
  vec4 color = texture(uBlocksTexture, vec3(vTexCoord, vTile));
  if(color.a < 0.5)
    discard;

  //gl_FragDepth = gl_FragCoord.z;
}