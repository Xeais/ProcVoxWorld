#include "Configuration.h"

#include "Utils.h"
#include "ini/ini.h"

//Macros for default values that appear multiple times in this file.
#define DEFAULT_CHUNK_RENDER_RADIUS 16
#define DEFAULT_BLOCK_BREAK_RADIUS 5
#define DEFAULT_CHUNK_WIDTH 32
#define DEFAULT_CHUNK_HEIGHT 256
#define DEFAULT_BLOCK_SIZE 0.1f

//General macros:
#define STRINGIFY(in) #in
#define CALL(macro, ...) macro(__VA_ARGS__) //An additional level of macro expansion ensures that also another macro can al be passed as an argument.

//--- WINDOW --- (default values)
int8_t* WINDOW_TITLE = "ProcVoxWorld"; //Should be changeable because of "TryToLoad()" or you get a "C4090" ("operation": different "modifier" qualifiers); this applies to "MAP_NAME" as well.

int32_t WINDOW_WIDTH  = 1280;
int32_t WINDOW_HEIGHT = 720;

bool FULLSCREEN = false;
bool VSYNC      = false;

//--- GRAPHICS --- (default values)
int32_t CHUNK_RENDER_RADIUS      = DEFAULT_CHUNK_RENDER_RADIUS; //High performance hit!
int32_t ANISOTROPIC_FILTER_LEVEL = 16; //Very low performance hitand a huge image quality boost in return

bool MOTION_BLUR_ENABLED    = true; //Motion blur implicates a low performance hit if the amount of samples is moderate.
float MOTION_BLUR_STRENGTH  = 0.0005f;
int32_t MOTION_BLUR_SAMPLES = 7;

bool DOF_ENABLED   = true; //Depth of field involves a medium performance hit if not a smooth depth of field is used.
bool DOF_SMOOTH    = true;
float DOF_MAX_BLUR = 0.023f;
float DOF_APERTURE = 0.3505f;
float DOF_SPEED    = 5.0f; //Affects only smooth depth of field.

int32_t FOV      = 65; //High performance hit if the field of view value is considerably enlarged.
int32_t FOV_ZOOM = 20;

float GAMMA      = 1.5f;
float SATURATION = 1.2f;

//--- GAMEPLAY --- (default values)
int32_t MAP_SEED = -1; /* -1 = random (Range for seed: 0 - RAND_MAX) | "RAND_MAX" is a constant defined in "<cstdlib>";
                                  *                                  | its value is library-dependent, but guaranteed to be at least 32767 on any standard library implementation. */
int8_t* MAP_NAME = "DefaultMap.db";

float MOUSE_SENS           = 0.1f;
int32_t BLOCK_BREAK_RADIUS = DEFAULT_BLOCK_BREAK_RADIUS; //Furthest distance (in blocks) for the player to reach.

int32_t DAY_LENGTH     = 1200; //Day length in seconds
bool DISABLE_TIME_FLOW = false; //Disable time, so there will always be day.

//Light multipliers for different day times:
float DAY_LIGHT     = 0.85f;
float EVENING_LIGHT = 0.5f;
float NIGHT_LIGHT   = 0.15f;

//--- CORE --- (default values)
int32_t NUM_WORKERS = 0; //Number of threads to use to load chunks. If zero, the number is automatically determined.

int32_t CHUNK_WIDTH  = DEFAULT_CHUNK_WIDTH; //Very delicate!
int32_t CHUNK_HEIGHT = DEFAULT_CHUNK_HEIGHT; //Very sensitive, too!
float BLOCK_SIZE     = DEFAULT_BLOCK_SIZE; /* Width of one block in GPU memory | If this value is too small, you will experience floating-point errors everywhere, but if it's too high,
                                            *                                  | there will be errors not far from the spawn location. */

/* --- PHYSICS--- (default values) 
 * All values are in blocks per second. */
float MAX_RUN_SPEED    = 5.612f;
float MAX_MOVE_SPEED   = 4.317f;
float MAX_SNEAK_SPEED  = 1.31f;
float MAX_SWIM_SPEED   = 3.5f;
float MAX_DIVE_SPEED   = 8.0f;
float MAX_EMERGE_SPEED = 10.0f;
float MAX_FALL_SPEED   = 57.46f;
float JUMP_POWER       = 8.3f;

//All values are in blocks per second squared.
float ACC_HORIZONTAL   = 40.0f;
float DEC_HORIZONTAL   = 40.0f;
float ACC_WATER_EMERGE = 20.0f;

float GRAVITY          = 27.44f;
float GRAVITY_WATER    = 9.14f;

//Internal values, that cannot or should not be set using an INI-file.
int32_t OPENGL_VERSION_MAJOR_REQUIRED = 4;
int32_t OPENGL_VERSION_MINOR_REQUIRED = 3; //Version 4.3 (release: August 6, 2012) is for "KHR_debug". Due to Direct State Access (DSA) at least OpenGL version 4.5 (release: August 11, 2014) would be required for the newer OpenGL variant.

float DAY_TO_EVE_START   = 0.3f;
float EVE_TO_NIGHT_START = 0.4f;
float NIGHT_START        = 0.45f;
float NIGHT_TO_DAY_START = 0.85f;

//The following values will be rectified (if necessary owing to values changed by the user) at the end of "ConfigurationLoad()".
int32_t CHUNK_RENDER_RADIUS_SQUARED = DEFAULT_CHUNK_RENDER_RADIUS * DEFAULT_CHUNK_RENDER_RADIUS;
int32_t CHUNK_LOAD_RADIUS           = DEFAULT_CHUNK_RENDER_RADIUS + 2;
int32_t CHUNK_LOAD_RADIUS_SQUARED   = (DEFAULT_CHUNK_RENDER_RADIUS + 2) * (DEFAULT_CHUNK_RENDER_RADIUS + 2);
int32_t CHUNK_UNLOAD_RADIUS         = DEFAULT_CHUNK_RENDER_RADIUS + 5;
int32_t CHUNK_UNLOAD_RADIUS_SQUARED = (DEFAULT_CHUNK_RENDER_RADIUS + 5) * (DEFAULT_CHUNK_RENDER_RADIUS + 5);
int32_t BLOCK_BREAK_RADIUS_SQUARED  = DEFAULT_BLOCK_BREAK_RADIUS * DEFAULT_BLOCK_BREAK_RADIUS;

float CHUNK_SIZE             = DEFAULT_CHUNK_WIDTH * DEFAULT_BLOCK_SIZE;
int32_t CHUNK_WIDTH_REAL     = DEFAULT_CHUNK_WIDTH + 2;
int32_t CHUNK_HEIGHT_REAL    = DEFAULT_CHUNK_HEIGHT + 2;
uintmax_t BLOCKS_MEMORY_SIZE = 0;

GLfloat const DEFAULT_CLEAR_COLOR[] = {1.0f, 0.8f, 0.6f, 1.0f};
//Newer OpenGL: GLfloat const CLEAR_DEPTH = 1.0f;

static void CreateDefaultConfigFile(const char* configPath)
{
  char content[] = "; Configuration file for ProcVoxWorld\n"
                   "; If there is a problem with this file, delete it and just launch the game;\n"
                   "; thereby a default configuration file will be created.\n\n"

                   "[WINDOW]\n"
                   "Title = ProcVoxWorld\n\n"

                   "Width  = 1280\n"
                   "Height = 720\n\n"

                   "Fullscreen = false\n\n"

                   "VSync = false ; Limits FPS to the maximum refresh rate of the monitor.\n\n\n"
  
                   "[GRAPHICS]\n"
                   "ChunkRenderRadius = 16 ; High performance hit!\n\n"
   
                   "AnisotropicFilterLevel = 16 ; Very low performance hit and a huge image quality boost in return\n\n"
    
                   "; Low performance hit if the amount of samples is moderate.\n"
                   "MotionBlurEnabled  = true\n"
                   "MotionBlurStrength = 0.0035\n"
                   "MotionBlurSamples  = 12\n\n"
   
                   "; Medium performance hit if not a smooth depth of field is used.\n"
                   "DepthOfFieldEnabled  = true\n"
                   "DepthOfFieldSmooth   = true\n"
                   "DepthOfFieldMaxBlur  = 0.023\n"
                   "DepthOfFieldAperture = 0.3505\n"
                   "DepthOfFieldSpeed    = 5.0 ; Affects only smooth depth of field.\n\n"
     
                   "; High performance hit if the field of view value is considerably enlarged.\n"
                   "FOV     = 75\n"
                   "FOVZoom = 20\n\n"
     
                   "Gamma      = 1.5\n"
                   "Saturation = 1.2\n\n\n"
    
                   "[GAMEPLAY]\n"
                   "MapSeed = -1 ; -1 = random (Range for seed: 0 - RAND_MAX)\n"
                   "MapName = DefaultMap.db\n\n"

                   "MouseSens = 0.1\n\n"
    
                   "BlockBreakRadius = 5 ; Furthest distance (in blocks) for the player to reach.\n\n"
    
                   "DayLength       = 1200 ; Day length in seconds\n"
                   "DisableTimeFlow = false ; Disable time so that it is always daytime.\n\n"
   
                   "; Light multipliers for different day times:\n"
                   "DayLight     = 0.85\n"
                   "EveningLight = 0.5\n"
                   "NightLight   = 0.15\n\n\n"
    
                   "[CORE]\n"
                   "; Use this number of threads to load chunks. If zero, the number is automatically determined.\n"
                   "; This can be delicate, so there is an upper bound!\n"
                   "NumWorkers = 0\n\n"
   
                   "; Chunk sizes:\n"
                   "ChunkWidth  = 32 ; Very delicate!\n"
                   "ChunkHeight = 256 ; Very sensitive, too!\n\n"
   
                   "; If this value is too small, you will experience floating-point errors everywhere,\n"
                   "; but if it's too high, there will be errors not far from the spawn location.\n"
                   "blockSize = 0.1 ; Width of one block in GPU memory\n\n\n"

                   "[PHYSICS]\n"
                   "; All values are in blocks per second.\n"
                   "MaxRunSpeed   = 5.612\n"
                   "MaxMoveSpeed  = 4.317\n"
                   "MaxSneakSpeed = 1.31\n\n"

                   "MaxSwimSpeed   = 3.5\n"
                   "MaxDiveSpeed   = 8.0\n"
                   "MaxEmergeSpeed = 10.0\n\n"

                   "MaxFallSpeed = 57.46\n"
                   "JumpPower    = 8.3\n\n"

                   "; All values are in blocks per second squared.\n"
                   "AccHorizontal  = 40.0\n"
                   "DecHorizontal  = 40.0\n"
                   "AccWaterEmerge = 20.0\n\n"

                   "Gravity      = 27.44\n"
                   "GravityWater = 9.14";

  //This is all just to replace the string "RAND_MAX" in the string "content" with the decimal value of the macro "RAND_MAX".  
  char replace[20];
  uintmax_t randMaxDecimal = HexToDecimal(CALL(STRINGIFY, RAND_MAX));
  sprintf_s(replace, ARRAY_SIZE(replace), "%" PRIuMAX, randMaxDecimal);
  StringReplace("RAND_MAX", replace, content);

  FILE* f = NULL;
  errno_t err = fopen_s(&f, configPath, "w");

  if(err != 0 || f == NULL)
  {
    char errMsg[94]; //-> https://developercommunity.visualstudio.com/t/strerrorlen-s-is-not-supported/160287#T-N625470
    strerror_s(errMsg, ARRAY_SIZE(errMsg), err);
    LogError("Could not create new configuration file \"%s\".\nError message from \"fopen_s()\": %s.", true, configPath, errMsg);

    return;
  }

  fprintf(f, "%s", content);
  fclose(f);

  LogSuccess("A new configuration file \"%s\" has been created.", true, configPath);
}

static void NormalizePlayerPhysics()
{
  MAX_RUN_SPEED *= BLOCK_SIZE;
  MAX_MOVE_SPEED *= BLOCK_SIZE;
  MAX_SNEAK_SPEED *= BLOCK_SIZE;
  MAX_SWIM_SPEED *= BLOCK_SIZE;
  MAX_DIVE_SPEED *= BLOCK_SIZE;
  MAX_EMERGE_SPEED *= BLOCK_SIZE;
  MAX_FALL_SPEED *= BLOCK_SIZE;
  JUMP_POWER *= BLOCK_SIZE;

  ACC_HORIZONTAL *= BLOCK_SIZE;
  DEC_HORIZONTAL *= BLOCK_SIZE;
  ACC_WATER_EMERGE *= BLOCK_SIZE;

  GRAVITY *= BLOCK_SIZE;
  GRAVITY_WATER *= BLOCK_SIZE;
}

static void PrintValueToStderr(const char* fmt, void* value, bool newLine)
{
  const char* nL = newLine ? "\n" : "";
  if(fmt == NULL)
    fprintf(stderr, "%s%s", *(const char**)value, nL);
  else if(!strcmp(fmt, "%d") /* Strings are identical. */)
    fprintf(stderr, "%d%s", *(int32_t*)value, nL);
  else
    fprintf(stderr, "%.3f%s", *(float*)value, nL); //Precision: three digits -> "printf - C++ Reference": https://www.cplusplus.com/reference/cstdio/printf/	
}

static void TryToLoad(ini_t* cfg, const char* section, const char* key, const char* fmt, void* dst)
{
  const char* tokenStr = ini_get(cfg, section, key);

  if(tokenStr == NULL)
  {
    //No log function was used here because their messages are self-contained, so ...
    if(STDERR_SUPPORTS_COLORS)
      fprintf(stderr, "%sParameter \"%s\" could not be loaded!\nTherefore, a default value is used: ", FG_YELLOW, key);
    else
      fprintf(stderr, "Parameter \"%s\" could not be loaded!\nTherefore, a default value is used: ", key);

    //... this part would be colorless in any case.
    PrintValueToStderr(fmt, dst, false);

    if(STDERR_SUPPORTS_COLORS)
      fprintf(stderr, "%s%s", FG_RESET, LOG_SEPARATOR);
    else
      fprintf(stderr, "%s", LOG_SEPARATOR);

    return;
  }

  if(fmt != NULL)
    sscanf_s(tokenStr, fmt, dst, strlen(tokenStr));
  else
    *(const char**)dst = OwnStrDup(tokenStr);
}

void ConfigurationLoad(const char* configPath)
{
  ini_t* cfg = ini_load(configPath);
  if(cfg != NULL)
    LogInfo("Loading settings from \"%s\" ...", true, configPath);
  else
  {
    LogError("Configuration file \"%s\" could not be found.", false, configPath);
    LogInfo("Hence, default settings are used and a new configuration file will be created.", true);

    CreateDefaultConfigFile(configPath);
    NormalizePlayerPhysics();

    return;
  }

  TryToLoad(cfg, "WINDOW", "Title", NULL, &WINDOW_TITLE);
  TryToLoad(cfg, "WINDOW", "Width", "%d", &WINDOW_WIDTH);
  TryToLoad(cfg, "WINDOW", "Height", "%d", &WINDOW_HEIGHT);

  TryToLoad(cfg, "WINDOW", "Fullscreen", "%d", &FULLSCREEN);
  TryToLoad(cfg, "WINDOW", "VSync", "%d", &VSYNC);

  TryToLoad(cfg, "GRAPHICS", "ChunkRenderRadius", "%d", &CHUNK_RENDER_RADIUS);
  TryToLoad(cfg, "GRAPHICS", "AnisotropicFilterLevel", "%d", &ANISOTROPIC_FILTER_LEVEL);

  TryToLoad(cfg, "GRAPHICS", "MotionBlurEnabled", "%d", &MOTION_BLUR_ENABLED);
  TryToLoad(cfg, "GRAPHICS", "MotionBlurStrength", "%f", &MOTION_BLUR_STRENGTH);
  TryToLoad(cfg, "GRAPHICS", "MotionBlurSamples", "%d", &MOTION_BLUR_SAMPLES);

  TryToLoad(cfg, "GRAPHICS", "DepthOfFieldEnabled", "%d", &DOF_ENABLED);
  TryToLoad(cfg, "GRAPHICS", "DepthOfFieldSmooth", "%d", &DOF_SMOOTH);
  TryToLoad(cfg, "GRAPHICS", "DepthOfFieldMaxBlur", "%f", &DOF_MAX_BLUR);
  TryToLoad(cfg, "GRAPHICS", "DepthOfFieldAperture", "%f", &DOF_APERTURE);
  TryToLoad(cfg, "GRAPHICS", "DepthOfFieldSpeed", "%f", &DOF_SPEED);

  TryToLoad(cfg, "GRAPHICS", "FOV", "%d", &FOV);
  TryToLoad(cfg, "GRAPHICS", "FOVZoom", "%d", &FOV_ZOOM);

  TryToLoad(cfg, "GRAPHICS", "Gamma", "%f", &GAMMA);
  TryToLoad(cfg, "GRAPHICS", "Saturation", "%f", &SATURATION);

  TryToLoad(cfg, "GAMEPLAY", "MapSeed", "%d", &MAP_SEED);
  TryToLoad(cfg, "GAMEPLAY", "MapName", NULL, &MAP_NAME);

  TryToLoad(cfg, "GAMEPLAY", "MouseSens", "%f", &MOUSE_SENS);
  TryToLoad(cfg, "GAMEPLAY", "BlockBreakRadius", "%d", &BLOCK_BREAK_RADIUS);

  TryToLoad(cfg, "GAMEPLAY", "DayLength", "%d", &DAY_LENGTH);
  TryToLoad(cfg, "GAMEPLAY", "DisableTimeFlow", "%d", &DISABLE_TIME_FLOW);

  TryToLoad(cfg, "GAMEPLAY", "DayLight", "%f", &DAY_LIGHT);
  TryToLoad(cfg, "GAMEPLAY", "EveningLight", "%f", &EVENING_LIGHT);
  TryToLoad(cfg, "GAMEPLAY", "NightLight", "%f", &NIGHT_LIGHT);

  TryToLoad(cfg, "CORE", "NumWorkers", "%d", &NUM_WORKERS);
  TryToLoad(cfg, "CORE", "ChunkWidth", "%d", &CHUNK_WIDTH);
  TryToLoad(cfg, "CORE", "ChunkHeight", "%d", &CHUNK_HEIGHT);
  TryToLoad(cfg, "CORE", "BlockSize", "%f", &BLOCK_SIZE);

  TryToLoad(cfg, "PHYSICS", "MaxRunSpeed", "%f", &MAX_RUN_SPEED);
  TryToLoad(cfg, "PHYSICS", "MaxMoveSpeed", "%f", &MAX_MOVE_SPEED);
  TryToLoad(cfg, "PHYSICS", "MaxSneakSpeed", "%f", &MAX_SNEAK_SPEED);
  TryToLoad(cfg, "PHYSICS", "MaxSwimSpeed", "%f", &MAX_SWIM_SPEED);
  TryToLoad(cfg, "PHYSICS", "MaxDiveSpeed", "%f", &MAX_DIVE_SPEED);
  TryToLoad(cfg, "PHYSICS", "MaxEmergeSpeed", "%f", &MAX_EMERGE_SPEED);
  TryToLoad(cfg, "PHYSICS", "MaxFallSpeed", "%f", &MAX_FALL_SPEED);
  TryToLoad(cfg, "PHYSICS", "JumpPower", "%f", &JUMP_POWER);

  TryToLoad(cfg, "PHYSICS", "AccHorizontal", "%f", &ACC_HORIZONTAL);
  TryToLoad(cfg, "PHYSICS", "DecHorizontal", "%f", &DEC_HORIZONTAL);
  TryToLoad(cfg, "PHYSICS", "AccWaterEmerge", "%f", &ACC_WATER_EMERGE);

  TryToLoad(cfg, "PHYSICS", "Gravity", "%f", &GRAVITY);
  TryToLoad(cfg, "PHYSICS", "GravityWater", "%f", &GRAVITY_WATER);

  NormalizePlayerPhysics();

  if(CHUNK_RENDER_RADIUS != DEFAULT_CHUNK_RENDER_RADIUS) //"CHUNK_RENDER_RADIUS" has been modified by the user via the INI-file and therefore needs to be recalculated.
  {
    CHUNK_RENDER_RADIUS_SQUARED = CHUNK_RENDER_RADIUS * CHUNK_RENDER_RADIUS;
    CHUNK_LOAD_RADIUS = CHUNK_RENDER_RADIUS + 2;
    CHUNK_LOAD_RADIUS_SQUARED = CHUNK_LOAD_RADIUS * CHUNK_LOAD_RADIUS;
    CHUNK_UNLOAD_RADIUS = CHUNK_RENDER_RADIUS + 5;
    CHUNK_UNLOAD_RADIUS_SQUARED = CHUNK_UNLOAD_RADIUS * CHUNK_UNLOAD_RADIUS;
  }

  if(BLOCK_BREAK_RADIUS != DEFAULT_BLOCK_BREAK_RADIUS) //Analog to "CHUNK_RENDER_RADIUS"
    BLOCK_BREAK_RADIUS_SQUARED = BLOCK_BREAK_RADIUS * BLOCK_BREAK_RADIUS;

  if(CHUNK_WIDTH != DEFAULT_CHUNK_WIDTH) //Analog to "CHUNK_RENDER_RADIUS"
  {
    if(BLOCK_SIZE != DEFAULT_BLOCK_SIZE) //Analog to "CHUNK_RENDER_RADIUS"
      CHUNK_SIZE = (float)CHUNK_WIDTH * BLOCK_SIZE;
    CHUNK_WIDTH_REAL = CHUNK_WIDTH + 2;
  }

  if(CHUNK_HEIGHT != DEFAULT_CHUNK_HEIGHT) //Analog to "CHUNK_RENDER_RADIUS"
    CHUNK_HEIGHT_REAL = CHUNK_HEIGHT + 2;

  BLOCKS_MEMORY_SIZE = (uintmax_t)CHUNK_WIDTH_REAL * CHUNK_WIDTH_REAL * CHUNK_HEIGHT_REAL;

  LogSuccess("Settings were successfully loaded from \"%s\".", true, configPath);

  ini_free(cfg);
}