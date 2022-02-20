#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aAO;
layout (location = 3) in uint aTile;
layout (location = 4) in uint aNormal;

out vec2 vTexCoord;
flat out uint vTile;

uniform mat4 MVPMatrix;

void main()
{
  gl_Position = MVPMatrix * vec4(aPos, 1.0);
  vTexCoord = aTexCoord;
  vTile = aTile;
}