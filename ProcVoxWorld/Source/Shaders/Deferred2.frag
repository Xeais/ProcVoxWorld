#version 430 core

/* ======================================= *
 * Motion Blur and Color Correction Shader *
 * ======================================= */

in vec2 vTexCoord;

out vec4 outColor;

uniform sampler2D textureColor;
uniform sampler2D textureUI;
uniform sampler2D textureDepth;

uniform bool uMotionBlurEnabled;
uniform mat4 uProjectionInvMat;
uniform mat4 uViewInvMat;
uniform mat4 uPrevViewMat;
uniform mat4 uProjectionMat;

uniform vec3 uCamPos;
uniform vec3 uPrevCamPos;
uniform float uStrength;
uniform int uSamples;
uniform float uDt;

uniform float uGamma;
uniform float uSaturation;

vec3 MotionBlur(vec3 RGB)
{
  float depth = texture(textureDepth, vTexCoord).r;
  vec4 currPos = vec4(vTexCoord, depth, 1.0) * 2.0 - 1.0;
   
  vec4 fragPos = uProjectionInvMat * currPos;
  fragPos = uViewInvMat * fragPos;
  fragPos /= fragPos.w;
  fragPos.xyz += uCamPos;

  vec4 fragPrevPos = fragPos;
  fragPrevPos.xyz -= uPrevCamPos;
  fragPrevPos = uPrevViewMat * fragPrevPos;
  fragPrevPos = uProjectionMat * fragPrevPos;
  fragPrevPos /= fragPrevPos.w;

  vec2 offset = (currPos - fragPrevPos).xy * uStrength / uDt;
  vec2 smallStep = offset / uSamples;
  vec2 coord = vTexCoord.xy + smallStep;

  vec3 color = RGB;
  for(int i = 1; i < uSamples; ++i, coord += smallStep) 
    color += texture(textureColor, coord).xyz;

  return color / uSamples;
}

vec3 GammaCorrection(vec3 RGB)
{
  return pow(outColor.rgb, vec3(1.0 / uGamma));
}

vec3 Saturation(vec3 RGB)
{
  const vec3 w = vec3(0.2125, 0.7154, 0.0721);
  vec3 intensity = vec3(dot(RGB, w));

  return mix(intensity, RGB, uSaturation);
}

void main()
{
  /* float d = texture(textureShadowDepth, vTexCoord).r;
   * outColor = vec4(vec3(d), 1.0);
   * return; */

  //Handle UI-elements:
  vec4 UIColor = texture(textureUI, vTexCoord);
  if(UIColor.a > 0.5)
  {
    outColor = UIColor;
    outColor.rgb = GammaCorrection(outColor.rgb);
    outColor.rgb = Saturation(outColor.rgb);

    return;
  }

  outColor = texture(textureColor, vTexCoord);
  outColor.a = 1.0;
 
  if(uMotionBlurEnabled)
    outColor.rgb = MotionBlur(outColor.rgb);

  outColor.rgb = GammaCorrection(outColor.rgb);
  outColor.rgb = Saturation(outColor.rgb);
}