#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

uniform mat4 MVPMatrix;

void main()
{
  vec4 pos = MVPMatrix * vec4(aPos, 1.0);
  gl_Position = pos.xyww;
  vTexCoord = aTexCoord;
}