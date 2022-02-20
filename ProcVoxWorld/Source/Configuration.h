#pragma once

#include "glad/glad.h" 
#include "CLIFormat.h"

#include <stdint.h> 
#include <stdbool.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array)) //The expression is evaluated at compile time, no runtime overhead.

//--- WINDOW ---
extern int8_t* WINDOW_TITLE;

extern int32_t WINDOW_WIDTH;
extern int32_t WINDOW_HEIGHT;

extern bool FULLSCREEN;
extern bool VSYNC;

//--- GRAPHICS ---
extern int32_t CHUNK_RENDER_RADIUS;
extern int32_t ANISOTROPIC_FILTER_LEVEL;

extern bool MOTION_BLUR_ENABLED;
extern float MOTION_BLUR_STRENGTH;
extern int32_t MOTION_BLUR_SAMPLES;

extern bool DOF_ENABLED;
extern bool DOF_SMOOTH;
extern float DOF_MAX_BLUR;
extern float DOF_APERTURE;
extern float DOF_SPEED;

extern int32_t FOV;
extern int32_t FOV_ZOOM;

extern float GAMMA;
extern float SATURATION;

//--- GAMEPLAY ---
extern int32_t MAP_SEED;
extern int8_t* MAP_NAME;

extern float MOUSE_SENS;
extern int32_t BLOCK_BREAK_RADIUS;

extern int32_t DAY_LENGTH;
extern bool DISABLE_TIME_FLOW;

extern float DAY_LIGHT;
extern float EVENING_LIGHT;
extern float NIGHT_LIGHT;

//--- CORE ---
extern int32_t NUM_WORKERS;

extern int32_t CHUNK_WIDTH;
extern int32_t CHUNK_HEIGHT;
extern float BLOCK_SIZE;

//--- PHYSICS ---
extern float MAX_RUN_SPEED;
extern float MAX_MOVE_SPEED;
extern float MAX_SNEAK_SPEED;
extern float MAX_SWIM_SPEED;
extern float MAX_DIVE_SPEED;
extern float MAX_EMERGE_SPEED;
extern float MAX_FALL_SPEED;
extern float JUMP_POWER;

extern float ACC_HORIZONTAL;
extern float DEC_HORIZONTAL;
extern float ACC_WATER_EMERGE;

extern float GRAVITY;
extern float GRAVITY_WATER;

//Internal values, that cannot or should not be set using an INI-file.
extern int32_t OPENGL_VERSION_MAJOR_REQUIRED;
extern int32_t OPENGL_VERSION_MINOR_REQUIRED;

extern float DAY_TO_EVE_START;
extern float EVE_TO_NIGHT_START;
extern float NIGHT_START;
extern float NIGHT_TO_DAY_START;

extern int32_t CHUNK_RENDER_RADIUS_SQUARED;
extern int32_t CHUNK_LOAD_RADIUS;
extern int32_t CHUNK_LOAD_RADIUS_SQUARED;
extern int32_t CHUNK_UNLOAD_RADIUS;
extern int32_t CHUNK_UNLOAD_RADIUS_SQUARED;
extern int32_t BLOCK_BREAK_RADIUS_SQUARED;

extern float CHUNK_SIZE;
extern int32_t CHUNK_WIDTH_REAL;
extern int32_t CHUNK_HEIGHT_REAL;
extern uintmax_t BLOCKS_MEMORY_SIZE;

extern const GLfloat DEFAULT_CLEAR_COLOR[];
//Newer OpenGL: extern const GLfloat CLEAR_DEPTH;

void ConfigurationLoad(const char* configPath);