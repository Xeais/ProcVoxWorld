#version 430 core

layout (location = 0) in vec3 aPos;

out vec3 vTexCoord;
out vec3 vTexCoordNight;
out vec3 vPos;

uniform mat4 MVPMatrix;
uniform float time;

#define PI 3.1415926535
#define COS60 0.5
#define SIN60 0.86602540378

void main()
{
  vec4 pos = MVPMatrix * vec4(aPos, 1.0);
  gl_Position = pos.xyww;
  vTexCoord = aPos;
  vPos = aPos;

  float sinT = sin(2 * PI * time);
  float cosT = cos(2 * PI * time);

  /* Rotate only the night texture.
   * This are two rotation matrices combined to one.
   * it rotates stars by 60 degrees on the x-axis and (360 * time) degrees on the y-axis. */
  vTexCoordNight.x = vTexCoord.x * cosT + vTexCoord.y * sinT * SIN60 + vTexCoord.z * sinT * COS60;
  vTexCoordNight.y = vTexCoord.y * COS60 - vTexCoord.z * SIN60;
  vTexCoordNight.z = -vTexCoord.x * sinT + vTexCoord.y * cosT * SIN60 + vTexCoord.z * cosT * COS60;
}