#version 430 core

in vec3 vTexCoord;
in vec3 vTexCoordNight;
in vec3 vPos;

out vec4 outColor;

uniform samplerCube textureDay;
uniform samplerCube textureEvening;
uniform samplerCube textureNight;

uniform float time;
uniform float dayToEveStart;
uniform float eveToNightStart;
uniform float nightStart;
uniform float nightToDayStart;

uniform vec3 fogColor;

void main()
{
  vec4 day = texture(textureDay, vTexCoord);
  vec4 evening = texture(textureEvening, vTexCoord);
  vec4 night = texture(textureNight, vTexCoordNight);

  //Blend different skyboxes according to current time.
  if(time < eveToNightStart)
    outColor = mix(day, evening, smoothstep(dayToEveStart, eveToNightStart, time));
  else if(time < nightToDayStart)
    outColor = mix(evening, night, smoothstep(eveToNightStart, nightStart, time));
  else
    outColor = mix(night, day, smoothstep(nightToDayStart, 1.0, time));

  vec3 dirPoint = normalize(vPos);
  vec3 projTo_XZ   = normalize(vec3(dirPoint.x, 0.0, dirPoint.z));
  float cosHorizon = dot(dirPoint, projTo_XZ);

  //The fog should appear slightly above the horizon as well as everywhere below it.
  float fogAmount = smoothstep(0.99, 1.0, cosHorizon);
  if(vPos.y < 0.0)
    fogAmount = 1.0;

  outColor = mix(outColor, vec4(fogColor, 1.0), fogAmount);
}