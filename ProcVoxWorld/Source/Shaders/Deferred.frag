#version 430 core

/* ===================== *
 * Depth of Field Shader *
 * ===================== */

in vec2 vTexCoord;

out vec4 outColor;

uniform sampler2D textureColor;
uniform sampler2D textureDepth;

uniform bool uDOFEnabled;
uniform bool uDOFSmooth;

uniform float uMaxBlur;
uniform float uDOFAperture;
uniform float uAspectRatio;
uniform float uDepth;

float DOFGetFactor()
{
  float depth = texture(textureDepth, vTexCoord).r;

  float centerDepth;
  if(!uDOFSmooth)
    centerDepth = texture(textureDepth, vec2(0.5, 0.5)).r;
  else
    centerDepth = uDepth;

  return centerDepth - depth;
}

vec3 DOF(vec3 RGB)
{
  float factor = DOFGetFactor();

  vec2 DOFBlur = vec2(clamp(factor * uDOFAperture, -uMaxBlur, uMaxBlur / 2.0));
  vec2 DOFBlur90P = DOFBlur * 0.9;
  vec2 DOFBlur70P = DOFBlur * 0.7;
  vec2 DOFBlur40P = DOFBlur * 0.4;

  vec2 aspectCorrect = vec2(1.0, uAspectRatio);

  vec4 col = vec4(RGB, 1.0);
  col += texture(textureColor, vTexCoord + (vec2(0.0, 0.4) * aspectCorrect) * DOFBlur);
  col += texture(textureColor, vTexCoord + (vec2(0.15, 0.37) * aspectCorrect) * DOFBlur);
  col += texture(textureColor, vTexCoord + (vec2(0.29, 0.29) * aspectCorrect) * DOFBlur);
  col += texture(textureColor, vTexCoord + (vec2(-0.37, 0.15) * aspectCorrect) * DOFBlur);
  col += texture(textureColor, vTexCoord + (vec2(0.40, 0.0) * aspectCorrect) * DOFBlur);
  col += texture(textureColor, vTexCoord + (vec2(0.37, -0.15) * aspectCorrect) * DOFBlur);
  col += texture(textureColor, vTexCoord + (vec2(0.29, -0.29) * aspectCorrect) * DOFBlur);
  col += texture(textureColor, vTexCoord + (vec2(-0.15, -0.37) * aspectCorrect) * DOFBlur);
  col += texture(textureColor, vTexCoord + (vec2(0.0, -0.4) * aspectCorrect) * DOFBlur);
  col += texture(textureColor, vTexCoord + (vec2(-0.15, 0.37) * aspectCorrect) * DOFBlur);
  col += texture(textureColor, vTexCoord + (vec2(-0.29, 0.29) * aspectCorrect) * DOFBlur);
  col += texture(textureColor, vTexCoord + (vec2(0.37, 0.15) * aspectCorrect) * DOFBlur);
  col += texture(textureColor, vTexCoord + (vec2(-0.4, 0.0) * aspectCorrect) * DOFBlur);
  col += texture(textureColor, vTexCoord + (vec2(-0.37, -0.15) * aspectCorrect) * DOFBlur);
  col += texture(textureColor, vTexCoord + (vec2(-0.29, -0.29) * aspectCorrect) * DOFBlur);
  col += texture(textureColor, vTexCoord + (vec2(0.15, -0.37) * aspectCorrect) * DOFBlur);

  col += texture(textureColor, vTexCoord + (vec2(0.15, 0.37) * aspectCorrect) * DOFBlur90P);
  col += texture(textureColor, vTexCoord + (vec2(-0.37, 0.15) * aspectCorrect) * DOFBlur90P);
  col += texture(textureColor, vTexCoord + (vec2(0.37, -0.15) * aspectCorrect) * DOFBlur90P);
  col += texture(textureColor, vTexCoord + (vec2(-0.15, -0.37) * aspectCorrect) * DOFBlur90P);
  col += texture(textureColor, vTexCoord + (vec2(-0.15, 0.37) * aspectCorrect) * DOFBlur90P);
  col += texture(textureColor, vTexCoord + (vec2(0.37, 0.15) * aspectCorrect) * DOFBlur90P);
  col += texture(textureColor, vTexCoord + (vec2(-0.37, -0.15) * aspectCorrect) * DOFBlur90P);
  col += texture(textureColor, vTexCoord + (vec2(0.15, -0.37) * aspectCorrect) * DOFBlur90P);

  col += texture(textureColor, vTexCoord + (vec2(0.29, 0.29) * aspectCorrect) * DOFBlur70P);
  col += texture(textureColor, vTexCoord + (vec2(0.40, 0.0) * aspectCorrect) * DOFBlur70P);
  col += texture(textureColor, vTexCoord + (vec2(0.29, -0.29) * aspectCorrect) * DOFBlur70P);
  col += texture(textureColor, vTexCoord + (vec2(0.0, -0.4) * aspectCorrect) * DOFBlur70P);
  col += texture(textureColor, vTexCoord + (vec2(-0.29, 0.29) * aspectCorrect) * DOFBlur70P);
  col += texture(textureColor, vTexCoord + (vec2(-0.4, 0.0) * aspectCorrect) * DOFBlur70P);
  col += texture(textureColor, vTexCoord + (vec2(-0.29, -0.29) * aspectCorrect) * DOFBlur70P);
  col += texture(textureColor, vTexCoord + (vec2(0.0, 0.4) * aspectCorrect) * DOFBlur70P);

  col += texture(textureColor, vTexCoord + (vec2(0.29, 0.29) * aspectCorrect) * DOFBlur40P);
  col += texture(textureColor, vTexCoord + (vec2(0.4, 0.0) * aspectCorrect) * DOFBlur40P);
  col += texture(textureColor, vTexCoord + (vec2(0.29, -0.29) * aspectCorrect) * DOFBlur40P);
  col += texture(textureColor, vTexCoord + (vec2(0.0, -0.4) * aspectCorrect) * DOFBlur40P);
  col += texture(textureColor, vTexCoord + (vec2(-0.29, 0.29) * aspectCorrect) * DOFBlur40P);
  col += texture(textureColor, vTexCoord + (vec2(-0.4, 0.0) * aspectCorrect) * DOFBlur40P);
  col += texture(textureColor, vTexCoord + (vec2(-0.29, -0.29) * aspectCorrect) * DOFBlur40P);
  col += texture(textureColor, vTexCoord + (vec2(0.0, 0.4) * aspectCorrect) * DOFBlur40P);

  col /= 41.0;

  return col.rgb;
}

void main()
{
  outColor = texture(textureColor, vTexCoord);
  outColor.a = 1.0;

  if(uDOFEnabled)
    outColor.rgb = DOF(outColor.rgb);
} 