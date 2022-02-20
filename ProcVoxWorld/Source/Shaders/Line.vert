#version 430 core

layout (location = 0) in vec3 aPos;

uniform mat4 MVPMatrix;

void main()
{
  gl_Position = MVPMatrix * vec4(aPos, 1.0);
}