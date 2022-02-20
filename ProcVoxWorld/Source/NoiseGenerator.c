#include "NoiseGenerator.h"

#include "Map/Map.h"

uint32_t OwnRand(uint32_t* prevValue)
{
  *prevValue = *prevValue * 1103515245 + 12345;

  return *prevValue;
}

noiseState* NoiseGeneratorCreateState(int32_t cX, int32_t cZ)
{
  noiseState* state = (noiseState*)OwnMalloc(sizeof(noiseState), false);

  if(state == NULL)
  {
    LogError("Variable \"state\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return NULL;
  }

  state->fnl = fnlCreateState();
  state->fnl.fractal_type = FNL_FRACTAL_FBM;
  state->fnl.cellular_distance_func = FNL_CELLULAR_DISTANCE_EUCLIDEAN;
  state->fnl.domain_warp_amp = 100.0f;
  state->fnl.domain_warp_type = FNL_DOMAIN_WARP_OPENSIMPLEX2;
  state->fnl.seed = MapGetSeed();

  state->randValue = (cX << 16) ^ cZ;

  return state;
}

void NoiseGeneratorSetSettings(fnl_state* state, fnl_noise_type noiseType, float freq, int32_t octaves, float lacunarity, float gain)
{
  state->noise_type = noiseType;
  state->frequency = freq;
  state->octaves = octaves;
  state->lacunarity = lacunarity;
  state->gain = gain;
}

//[0.0, 1.0]
float NoiseGenerator2D(fnl_state* state, float x, float z)
{
  return (fnlGetNoise2D(state, x, z) + 1.0f) / 2.0f;
}