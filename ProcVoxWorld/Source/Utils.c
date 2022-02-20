#include "Utils.h"

CPUInfo GetCPUInfo()
{
  CPUInfo result = {0};
  int32_t CPUInfo[4] = {-1};

  //CPU-brand (manufacturer, model and clock speed):
#ifdef PLATFORM_WINDOWS
  __cpuid(CPUInfo, 0x80000000);
#elif defined PLATFORM_POSIX
#if defined __GNUC__ || defined __GNUG__
  __cpuid(0x80000000, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
#endif
#endif

  char CPUBrandStr[0x40] = {0};
  char namelessCPU[] = "CPU without identifier";

  uint32_t exIds = CPUInfo[0];
  for(uint32_t i = 0x80000000; i <= exIds; ++i)
  {
#ifdef PLATFORM_WINDOWS
    __cpuid(CPUInfo, i);
#elif defined PLATFORM_POSIX && (defined __GNUC__ || defined __GNUG__)
    __cpuid(i, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
#endif

    //Interpret CPU-brand string:
    switch(i)
    {
      case 0x80000002:
        memcpy(CPUBrandStr, CPUInfo, sizeof(CPUInfo));
        break;
      case 0x80000003:
        memcpy(CPUBrandStr + 16, CPUInfo, sizeof(CPUInfo));
        break;
      case 0x80000004:
        memcpy(CPUBrandStr + 32, CPUInfo, sizeof(CPUInfo));
        break;
      default:
        if(strcmp(CPUBrandStr, "") == 0 && i == exIds) //No match was found!
          memcpy(CPUBrandStr, namelessCPU, strlen(namelessCPU) + 1);
        break;
    }
  }

  strcpy_s(result.CPUBrandString, ARRAY_SIZE(result.CPUBrandString), CPUBrandStr);

  //CPU cores:
  memset(CPUInfo, 0, sizeof(CPUInfo)); //Reset for reuse.
  uint32_t logProcCores;
  uint32_t phyProcCores;

#ifdef PLATFORM_WINDOWS
#if _WIN32_WINNT >= 0x0601 //-> "GetActiveProcessorCount function (winbase.h)": https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getactiveprocessorcount
  logProcCores = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
#else
  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);
  logProcCores = sysInfo.dwNumberOfProcessors;
#endif

  __cpuid(CPUInfo, 1);
#elif defined PLATFORM_POSIX
  logProcCores = sysconf(_SC_NPROCESSORS_ONLN);

#if defined __GNUC__ || defined __GNUG__
  __cpuid(1, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
#endif
#endif

  const int32_t CPUFeatureSet = CPUInfo[3];
  const bool HTT = CPUFeatureSet & (1 << 28); //Hyper-Threading Technology

  if(HTT)
    phyProcCores = logProcCores / 2;
  else
    phyProcCores = logProcCores;

  result.logicalProcCores = (int8_t)logProcCores;
  result.physicalProcCores = (int8_t)phyProcCores;
  result.hyperThreadingTech = HTT;

  return result;
}

const char* StringReplace(const char* search, const char* replace, int8_t* string)
{
  int8_t* tempString, * searchStart;
  ptrdiff_t len;
  size_t tempStringLen = strlen(string) + 1;

  //Is the search string present?
  searchStart = strstr(string, search);
  if(searchStart == NULL)
    return string;

  tempString = (int8_t*)OwnMalloc(tempStringLen * sizeof(int8_t), false);

  if(tempString == NULL)
  {
    LogError("Variable \"tempString\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return NULL;
  }

  strcpy_s(tempString, tempStringLen, string);

  len = searchStart - string;
  string[len] = '\0';

  strcat_s(string, len + 1 + strlen(replace), replace);

  len += strlen(search);
  int8_t* restStr = (int8_t*)(tempString + len);
  strcat_s(string, len + 1 + strlen(restStr), restStr);

  free(tempString);

  return string;
}

GLuint OpenGLCreateVAO()
{
  GLuint VAO;

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  //Newer OpenGL: glCreateVertexArrays(1, &VAO); //"glCreateVertexArrays()" (like other "glCreateX()" from DSA) also takes over the initialization, whereby "glBindVertexArray()" is rendered obsolete, however, only in this context.

  return VAO;
}

GLuint OpenGLCreateVBO(const void* vertices, GLsizeiptr bufSize)
{
  GLuint VBO;

  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, bufSize, vertices, GL_STATIC_DRAW);

 /* Newer OpenGL:
  * glCreateBuffers(1, &VBO);
  * glBindBuffer(GL_ARRAY_BUFFER, VBO); //Without this: Buffer object X is bound to "NONE".
  * glNamedBufferData(VBO, bufSize, vertices, GL_STATIC_DRAW); //void glNamedBufferData(GLuint buffer, GLsizeiptr size, const void* data, GLenum usage); */

  return VBO;
}

GLuint OpenGLCreateVBOCube()
{
  static const float vertices[] = 
  {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
  };

  return OpenGLCreateVBO(vertices, sizeof(vertices));
}

GLuint OpenGLCreateVBOQuad()
{
  static const float vertices[] = 
  {
    -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
     0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, 0.0f,  1.0f, 1.0f
  };

  return OpenGLCreateVBO(vertices, sizeof(vertices));
}

GLuint OpenGLCreateFBO()
{
  GLuint FBO;

  glGenFramebuffers(1, &FBO);
  glBindFramebuffer(GL_FRAMEBUFFER, FBO);

  //Newer OpenGL: glCreateFramebuffers(1, &FBO);
 
  return FBO;
}