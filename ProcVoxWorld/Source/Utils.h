#pragma once

/* For newer OpenGL:
 * #define GLFW_INCLUDE_NONE
 * #include "GLFW/glfw3.h" */

#include "cglm/cglm.h"

#include "Configuration.h"

#include "Log.h"

#include <inttypes.h>

/* Detects all Windows 64-bit versions with the compilers (likewise the 64-bit versions) Microsoft Visual Studio, Clang/LLVM (Windows and MinGW Target), 
 * GNU GCC/G++ (Windows and MinGW Target), Intel ICC/ICPC and Portland PGCC/PGCPP. */
#if defined _WIN64
#define PLATFORM_WINDOWS
#define PATH_SLASHES '//'
#include "Windows.h"
#elif defined _POSIX_VERSION //POSIX and UNIX are not operating systems. Rather they are formal or de facto standards followed to some degree by all UNIX-style OSes.
#define PLATFORM_POSIX
#define PATH_SLASHES '\\'
#include "unistd.h"
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct
{
  char CPUBrandString[70];

  uint32_t physicalProcCores;
  uint32_t logicalProcCores;

  bool hyperThreadingTech;
} CPUInfo;

//Vertex layout for storing block data in GPU
typedef struct
{
  float pos[3];
  float texCoord[2];
  float AO;

  uint8_t tile;
  uint8_t normal;
} Vertex;

CPUInfo GetCPUInfo();

const char* StringReplace(const char* search, const char* replace, int8_t* string);

GLuint OpenGLCreateVAO();

GLuint OpenGLCreateVBO(const void* vertices, GLsizeiptr bufSize);

GLuint OpenGLCreateVBOCube();

GLuint OpenGLCreateVBOQuad();

GLuint OpenGLCreateFBO();

//----- Inline -----

static inline void* OwnMalloc(size_t size, bool printDebugInfo)
{
  void* result = malloc(size); //-> "Difference between malloc and calloc?": https://stackoverflow.com/a/25344310

  const char* what = size != 1 ? "bytes" : "byte";

  if(result == NULL)
  {
    if(printDebugInfo)
      LogError("The allocation of a %zu %s memory block failed.", true, size, what);

    return NULL;
  }

  /* 1. "memset()" is absolutely necessary; otherwise there are in no way	negligible problems with the procedural generation. 
   * 2. Here is "memset()" without constraint responsible for the appearance of a clear bottleneck as it pulls in (https://stackoverflow.com/a/59196652) so much memory 
        that the graphics card does not receive any more data. 
   * 3. This problem persists in debug mode; To ensure debugging, even with this constrictive "memset()", all necessary memory is accessed, resulting in very high RAM usage. 
   *    During execution, this will initially yield a loading time (up to a few minutes); meanwhile, the GPU remains idle due to the lack of data reception, 
   *    which leads to a miserable frame rate. After the initial loading phase, the frame rate normalizes, though, when new chunks are created, the problem recurs 
   *    causing short reload stutters. */
  if(size <= BLOCKS_MEMORY_SIZE)
    memset(result, 0, size);
  else if(printDebugInfo)
    LogInfo("\"size\" (%zu) in function \"%s\" was larger than the limit (%" PRIuMAX "), wherefore \"memset()\" was skipped.", true, size, __func__, BLOCKS_MEMORY_SIZE);

  if(printDebugInfo)
    LogInfo("A memory block of %zu %s has been allocated.", true, size, what);

  return result;
}

static inline uintmax_t HexToDecimal(const char* hex)
{
  uintmax_t decimal = 0, base = 1;
  int32_t length = (int32_t)strlen(hex);
  for(int32_t i = --length; i >= 0; --i)
  {
    if(hex[i] >= '0' && hex[i] <= '9')
    {
      decimal += ((uintmax_t)hex[i] - 48) * base;
      base *= 16;
    }
    else if(hex[i] >= 'A' && hex[i] <= 'F')
    {
      decimal += ((uintmax_t)hex[i] - 55) * base;
      base *= 16;
    }
    else if(hex[i] >= 'a' && hex[i] <= 'f')
    {
      decimal += ((uintmax_t)hex[i] - 87) * base;
      base *= 16;
    }
  }

  return decimal;
}

/* Get next smallest value that is divisible by eight:
 * 5 -> 0, 9 -> 8, -1 -> -8, -8 -> -8, ... */
static inline int32_t FloorEight(int32_t a)
{
  if(a >= 0)
    return (int32_t)(a / 8) * 8;
  else
    return ((int32_t)((a + 1) / 8) - 1) * 8;
}

/* Returns the filename portion of the given path.
 * If "path" is only a directory, an empty string is returned. */
static inline int8_t* GetFilename(const char* path)
{
  int8_t* filename = (int8_t*)strrchr(path, PATH_SLASHES);
  if(filename == NULL)
    filename = (int8_t*)path;
  else
    ++filename;

  return filename;
}

static inline uint32_t GetProcessorsCount()
{
#ifdef PLATFORM_WINDOWS
#if _WIN32_WINNT >= 0x0601
  return GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
#else
  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);

  return sysInfo.dwNumberOfProcessors;
#endif
#elif defined PLATFORM_POSIX
  return sysconf(_SC_NPROCESSORS_ONLN);
#else
  //Enough plattform support for now!
  return 1;
#endif
}

static inline void OwnGLMVec3Set(vec3 vec, float f0, float f1, float f2)
{
  vec[0] = f0;
  vec[1] = f1;
  vec[2] = f2;
}

static inline void OwnGLMIvec3Set(ivec3 vec, int32_t i0, int32_t i1, int32_t i2)
{
  vec[0] = i0;
  vec[1] = i1;
  vec[2] = i2;
}

static inline float LoopBetween(float var, float min, float max)
{
  if(var > max)
    return min + (var - max);

  if(var < min)
    return max - (min - var);

  return var;
}

static inline int32_t ChunkedBlock(int32_t worldBlockCoord)
{
  if(worldBlockCoord >= 0)
    return worldBlockCoord / CHUNK_WIDTH;

  return (worldBlockCoord + 1) / CHUNK_WIDTH - 1;
}

static inline int32_t ToChunkCoord(int32_t worldBlockCoord)
{
  int32_t block = worldBlockCoord % CHUNK_WIDTH;
  if(block < 0) 
    block += CHUNK_WIDTH;

  return block;
}

static inline int32_t ChunkedCam(float camCoord)
{
  return (int32_t)(camCoord / CHUNK_SIZE);
}

static inline float Blocked(float coord)
{
  return coord / BLOCK_SIZE;
}

static inline int32_t ChunkPlayerDistSquared(int32_t cX, int32_t cZ, int32_t pX, int32_t pZ)
{
  return (cX - pX) * (cX - pX) + (cZ - pZ) * (cZ - pZ);
}

static inline float BlockPlayerDistSquared(int32_t bX, int32_t bY, int32_t bZ, float pX, float pY, float pZ)
{
  return (bX - pX) * (bX - pX) + (bY - pY) * (bY - pY) + (bZ - pZ) * (bZ - pZ);
}

static inline void OpenGL_VBOLayout(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, size_t offset)
                                 /* Newer OpenGL:
                                  * GLuint VAO, GLuint VBO, GLuint attribIndex, GLuint bindingIndex, GLint size,
                                  * GLenum type, GLboolean normalized, GLuint offset, GLsizei stride */
{
  if(type >= GL_BYTE && type <= GL_UNSIGNED_INT)
    glVertexAttribIPointer(index, size, type, stride, (const void*)offset);
  else
    glVertexAttribPointer(index, size, type, normalized, stride, (const void*)offset);

  glEnableVertexAttribArray(index);

  /* Newer OpenGL:
   * if(stride == 0)
   * {
   *   LogError("The stride parameter must always be specified when \"glVertexArrayVertexBuffer()\" is used!", true);
   *
   *   //This could be called from anywhere, so checking that GLFW initialized successfully makes sense.
   *   if(glfwInit()) //Additional calls to "glfwInit()" after successful initialization but before termination will return "GL_TRUE" immediately.
   *     glfwTerminate();
   *
   *   exit(EXIT_FAILURE); //Failing to provide a stride for "glVertexArrayVertexBuffer()" can result in really nasty situations - one example : https://stackoverflow.com/q/65519444. 
   *                       //Unfortunately, the documentation fails to mention this fact.
   * }
   *
   * glEnableVertexArrayAttrib(VAO, attribIndex);
   *
   * if(type >= GL_BYTE && type <= GL_UNSIGNED_INT)
   *   glVertexArrayAttribIFormat(VAO, attribIndex, size, type, offset);
   * else
   *   glVertexArrayAttribFormat(VAO, attribIndex, size, type, normalized, offset);
   *
   * //Attach a buffer to be read from.
   * glVertexArrayVertexBuffer(VAO, bindingIndex, VBO, (GLintptr)offset, stride);
   *
   * glVertexArrayAttribBinding(VAO, attribIndex, bindingIndex); */
}

static inline const char* OwnStrDup(const char* src)
{
  const size_t bufSize = strlen(src) + 1;
  const char* newStr = (const char*)OwnMalloc(bufSize, false);

  if(newStr != NULL)
    memcpy((void*)newStr, src, bufSize);
  else
  {
    LogError("Variable \"newStr\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return "";
  }

  return newStr;
}

//Almost like "glm_aabb_aabb()", but ">=" resp. "<=" are replaced with ">" and "<".
static inline int32_t AABBCollide(vec3 box[2], vec3 other[2])
{
  return (box[0][0] < other[1][0] && box[1][0] > other[0][0]) &&
         (box[0][1] < other[1][1] && box[1][1] > other[0][1]) &&
         (box[0][2] < other[1][2] && box[1][2] > other[0][2]);
}