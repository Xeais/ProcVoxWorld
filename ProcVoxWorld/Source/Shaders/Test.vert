#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

uniform mat4 MVPMatrix;

void main()
{
  gl_Position = MVPMatrix * vec4(aPos, 1.0);
  vTexCoord = aTexCoord;
}