#pragma once

#include "FastNoiseLite/FastNoiseLite.h"

//Simple thread-safe "rand()":
uint32_t OwnRand(uint32_t* prevValue);

typedef struct
{
  fnl_state fnl;
  uint32_t randValue;
} noiseState;

noiseState* NoiseGeneratorCreateState(int32_t cX, int32_t cZ);

void NoiseGeneratorSetSettings(fnl_state* state, fnl_noise_type noiseType, float freq, int32_t octaves, float lacunarity, float gain);

float NoiseGenerator2D(fnl_state* state, float x, float z);