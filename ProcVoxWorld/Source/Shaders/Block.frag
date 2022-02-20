#version 430 core

in vec3 vPos;
in vec2 vTexCoord;
in float vAO;
flat in uint vTile;
in float vFogAmount;
in vec3 vNormal;

out vec4 outColor;

uniform sampler2DArray textureSampler;
uniform float blockLight;
uniform vec3 fogColor;

//--------------------------------------
in vec4 vNearShadowMapCoord;
in vec4 vFarShadowMapCoord;

uniform sampler2DShadow uNearShadowMap;
uniform sampler2DShadow uFarShadowMap;

uniform vec3 uLightDir;
uniform float uNearShadowDist;
uniform float uShadowBlendDist;
uniform vec3 uPlayerPos;

/* There is only one uniform variable in more than two shaders ("MVPMatrix" is in seven) and this is thoroughly unsuitable as a Uniform Buffer Object (UBO), 
 * since the model matrix ("MVPMatrix" stands for "ModelViewProjectionMatrix") tends to be frequently changing between shaders; conclusion: UBOs don't fit well here.
 * ------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * Beware: If an uniform is declared that isn't used anywhere in the GLSL-code, the compiler will silently remove the variable from the compiled version; 
 * tnis can be the cause for really frustrating errors! */

uniform mat4 uNearLightMatrix;
uniform mat4 uFarLightMatrix;

uniform float uShadowMultiplier;
//--------------------------------------

vec2 poissonDisk[16] = vec2[](vec2(-0.94201624, -0.39906216), 
                              vec2( 0.94558609, -0.76890725), 
                              vec2(-0.094184101, -0.92938870), 
                              vec2( 0.34495938, 0.29387760), 
                              vec2(-0.91588581, 0.45771432), 
                              vec2(-0.81544232, -0.87912464), 
                              vec2(-0.38277543, 0.27676845), 
                              vec2( 0.97484398, 0.75648379), 
                              vec2( 0.44323325, -0.97511554), 
                              vec2( 0.53742981, -0.47373420), 
                              vec2(-0.26496911, -0.41893023), 
                              vec2( 0.79197514, 0.19090188), 
                              vec2(-0.24188840, 0.99706507), 
                              vec2(-0.81409955, 0.91437590), 
                              vec2( 0.19984126, 0.78641367), 
                              vec2( 0.14383161, -0.14100790));

//Returns a Random number based on a "vec3" and an "int".
float Random(vec3 seed, int i)
{
  vec4 seed4 = vec4(seed, i);
  float dotProduct = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
  
  return fract(sin(dotProduct) * 43758.5453);
}

float CalculateShadow(vec4 shadowMapCoord, sampler2DShadow shadowMap, float diskDividor)
{
  vec3 projCoords = shadowMapCoord.xyz / shadowMapCoord.w;
  projCoords = (projCoords + 1.0) / 2.0;

  if(projCoords.z > 1.0)
    return 0.0;

  float shadowFactor = 0.0;
  for(int i = 0; i < 9; ++i) 
  {
    int index = int(16.0 * Random(vec3(vTile, vNormal.y, vAO), i)) % 16;
    shadowFactor += texture(shadowMap, vec3(projCoords.xy + poissonDisk[index] / diskDividor, projCoords.z));
  }
  shadowFactor /= 9.0;

  return 1.0 - shadowFactor;
}

void main()
{    
  vec4 color = texture(textureSampler, vec3(vTexCoord, vTile));
  if(color.a < 0.5)
    discard;

  float shadowFactor = 0.0;

  //Normal check:
  if(length(vNormal) < 1.2 && dot(uLightDir, vNormal) >= 0) 
    shadowFactor = 1.0;
  else
  {
    float fragDist = distance(uPlayerPos, vPos);
    if(fragDist < uNearShadowDist - uShadowBlendDist)
      shadowFactor = CalculateShadow(vNearShadowMapCoord, uNearShadowMap, 3000.0);
    else if(fragDist > uNearShadowDist + uShadowBlendDist)
      shadowFactor = CalculateShadow(vFarShadowMapCoord, uFarShadowMap, 1500.0);
    else
    {
      float shadowNear = CalculateShadow(vNearShadowMapCoord, uNearShadowMap, 3000.0);
      float shadowFar  = CalculateShadow(vFarShadowMapCoord,  uFarShadowMap,  1500.0);

      float mixFactor = (fragDist - (uNearShadowDist - uShadowBlendDist)) / (2.0 * uShadowBlendDist);
      shadowFactor = mix(shadowNear, shadowFar, mixFactor);
    }
  }

  //Smooth shadowing in tight angles:
  float cosAngle = max(0.0, dot(-uLightDir, vNormal));
  float start = 0.2;
  shadowFactor += (1.0 - smoothstep(0.0, start, cosAngle));

  shadowFactor *= uShadowMultiplier;
  shadowFactor = clamp(shadowFactor, 0.0, 1.0);

  color.rgb *= (1.0 - shadowFactor / 2.0);
  color.a += shadowFactor / 3.0;

  color.rgb -= 0.35 * vAO;
  color.rgb *= blockLight;

  color.rgb = mix(color.rgb, fogColor, vFogAmount);

  outColor = color;
}