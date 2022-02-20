#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aAO;
layout (location = 3) in uint aTile;
layout (location = 4) in uint aNormal;

out vec3 vPos;
out vec2 vTexCoord;
out float vAO;
flat out uint vTile;
out float vFogAmount;
out vec3 vNormal;

uniform mat4 MVPMatrix;
uniform vec3 camPos;
uniform float fogDist;
uniform vec3 uLightDir;

//------------------------------
out vec4 vNearShadowMapCoord;
out vec4 vFarShadowMapCoord;

uniform mat4 uNearShadowMapMat;
uniform mat4 uFarShadowMapMat;
//------------------------------

const vec3 normals[7] = vec3[](vec3(-1.0,  0.0,  0.0),  //0: Left
                               vec3( 1.0,  0.0,  0.0),  //1: Right
                               vec3( 0.0,  1.0,  0.0),  //2: Top
                               vec3( 0.0, -1.0,  0.0),  //3: Bottom
                               vec3( 0.0,  0.0, -1.0),  //4: Back
                               vec3( 0.0,  0.0,  1.0),  //5: Front
                               vec3( 1.0,  1.0,  1.0)); //6: Undefined

void main()
{
  gl_Position = MVPMatrix * vec4(aPos, 1.0);
  vPos = aPos;
  vAO = aAO;
  vTile = aTile;
  vTexCoord = aTexCoord;

  float distToCam = distance(camPos.xz, aPos.xz);
  vFogAmount = pow(clamp(distToCam / fogDist, 0.0, 1.0), 4.0);

  vNearShadowMapCoord = uNearShadowMapMat * vec4(aPos, 1.0);
  vFarShadowMapCoord = uFarShadowMapMat * vec4(aPos, 1.0);

  vNormal = normals[aNormal];
}