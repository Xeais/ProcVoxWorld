#include "Shader.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "Utils.h"
#include "Texture.h"

GLuint SHADER_BLOCK;
GLuint SHADER_LINE;
GLuint SHADER_SKYBOX;
GLuint SHADER_SUN;
GLuint SHADER_DEFERRED;
GLuint SHADER_DEFERRED2;
GLuint SHADER_SHADOW;
GLuint SHADER_PIP;
GLuint SHADER_HAND_ITEM;

static int8_t* GetFileData(const char* path)
{
  FILE* f = NULL;
  errno_t err = fopen_s(&f, path, "rb");

  if(err != 0 || f == NULL)
  {
    char errMsg[94];
    strerror_s(errMsg, ARRAY_SIZE(errMsg), err);
    LogError("Shader file \"%s\" could not be opened.\nError message from \"fopen_s()\": %s", true, path, errMsg);
    glfwTerminate();

    exit(EXIT_FAILURE);
  }

  fseek(f, 0, SEEK_END);
  size_t dataSize = ftell(f);
  fseek(f, 0, SEEK_SET);

  int8_t* fileContent = (int8_t*)OwnMalloc(dataSize + 1 /* "+ 1" is for the null termination ("fileContent[dataSize] = '\0';") a bit later. */, false);

  if(fileContent == NULL)
  {
    LogError("Variable \"fileContent\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return NULL;
  }

  fread(fileContent, 1, dataSize, f);
  fileContent[dataSize] = '\0';

  fclose(f);

  return fileContent;
}

static GLuint CompileShader(const char* path, GLenum shaderType)
{
  int8_t* shaderSrc = GetFileData(path);

  if(shaderSrc == NULL)
  {
    LogError("Variable \"shaderSrc\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return 0;
  }

  /* Program pipeline objects allow to change shader stages on the fly without having to relink them. It is not used here 
   * since Nvidia drivers prefer monolithic shader programs or they may show poor performance. 
   * 
   * OpenGL-functions: 
   * - glCreateShaderProgramv()
   * - glCreateProgramPipelines()
   * - glUseProgramStages()
   * - glBindProgramPipeline() */

  GLuint shaderID = glCreateShader(shaderType);
  glShaderSource(shaderID, 1, (const GLchar**)&shaderSrc, NULL);
  glCompileShader(shaderID);

  free(shaderSrc);

  GLint success;
  glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);

  if(success)
    LogSuccess("Compilation of shader \"%s\" succeeded.", true, path);
  else
  {
    char info[512];
    glGetShaderInfoLog(shaderID, ARRAY_SIZE(info), NULL, info);
    info[strcspn(info, "\n")] = '\0';
    LogError("Compilation of shader \"%s\" failed!\nInformation log: %s", true, path, info);
    glfwTerminate();

    exit(EXIT_FAILURE);
  }

  return shaderID;
}

static GLuint CreateShaderProgram(const char* VSPath, const char* FSPath)
{
  GLuint VS_ID = CompileShader(VSPath, GL_VERTEX_SHADER);
  GLuint FS_ID = CompileShader(FSPath, GL_FRAGMENT_SHADER);
  if(!VS_ID || !FS_ID)
  {
    LogError("The compiled vertex (%u) and/or fragment shader (%u) couldn't be retrieved in the beginning of the shader program creation.", true, VS_ID, FS_ID);
    glfwTerminate();

    exit(EXIT_FAILURE);
  }

  const char* VSFilename = GetFilename(VSPath);
  const char* FSFilename = GetFilename(FSPath);

  GLuint shaderProg = glCreateProgram();
  glAttachShader(shaderProg, VS_ID);
  glAttachShader(shaderProg, FS_ID);
  glLinkProgram(shaderProg);

  GLint success;
  glGetProgramiv(shaderProg, GL_LINK_STATUS, &success);

  if(success)
    LogSuccess("Shader program was successfully linked.", true);
  else
  {
    char info[512];
    glGetProgramInfoLog(shaderProg, ARRAY_SIZE(info), NULL, info);
    LogError("Shader program could not be linked!\nInformation log: %s", true, info);
    glfwTerminate();

    exit(EXIT_FAILURE);
  }

  glDeleteShader(VS_ID);
  glDeleteShader(FS_ID);

  LogInfo("\"%s\" and \"%s\" belong to shader program %u.", true, VSFilename, FSFilename, shaderProg);

  return shaderProg;
}

/* Shader file extensions
 * ииииииииииииииииииииии
 *   - .vert = a vertex shader
 *   - .tesc = a tessellation control shader
 *   - .tese = a tessellation evaluation shader
 *   - .geom = a geometry shader
 *   - .frag = a fragment shader
 *   - .comp = a compute shader */
void ShaderInitAll()
{
  SHADER_BLOCK = CreateShaderProgram("Source/Shaders/Block.vert", "Source/Shaders/Block.frag");

  SHADER_LINE = CreateShaderProgram("Source/Shaders/Line.vert", "Source/Shaders/Line.frag");

  SHADER_SKYBOX = CreateShaderProgram("Source/Shaders/Skybox.vert", "Source/Shaders/Skybox.frag");

  SHADER_SUN = CreateShaderProgram("Source/Shaders/Sun.vert", "Source/Shaders/Sun.frag");

  SHADER_DEFERRED = CreateShaderProgram("Source/Shaders/Deferred.vert", "Source/Shaders/Deferred.frag");

  SHADER_DEFERRED2 = CreateShaderProgram("Source/Shaders/Deferred2.vert", "Source/Shaders/Deferred2.frag");

  SHADER_SHADOW = CreateShaderProgram("Source/Shaders/Shadow.vert", "Source/Shaders/Shadow.frag");

  SHADER_PIP = CreateShaderProgram("Source/Shaders/PiP.vert", "Source/Shaders/PiP.frag");

  //Not used at the moment: SHADER_HAND_ITEM = CreateShaderProgram("Source/Shaders/HandItem.vert", "Source/Shaders/HandItem.frag");
}

static GLint GetAttribLocation(GLuint shaderProgram, const char* name)
{
  GLint location = glGetUniformLocation(shaderProgram, name);
  if(location == -1)
  {
    LogWarning("\"%s\" does not correspond to an active uniform variable.\nOther possibilities:\n"
                                                                          "  - It starts with the reserved prefix \"gl_\".\n"
                                                                          "  - It is associated with an atomic counter or a named uniform block.", true, name);
  }

  return location;
}

void ShaderSetInt1(GLuint shaderProgram, const char* name, GLint value)
{
  glUniform1i(GetAttribLocation(shaderProgram, name), value);
  //Newer OpenGL: glProgramUniform1i(shaderProgram, GetAttribLocation(shaderProgram, name), value);
}

void ShaderSetFloat1(GLuint shaderProgram, const char* name, GLfloat value)
{
  glUniform1f(GetAttribLocation(shaderProgram, name), value);
  //Newer OpenGL: glProgramUniform1f(shaderProgram, GetAttribLocation(shaderProgram, name), value);
}

void ShaderSetFloat3(GLuint shaderProgram, const char* name, vec3 vec)
{
  glUniform3f(GetAttribLocation(shaderProgram, name), vec[0], vec[1], vec[2]);
  //Newer OpenGL: glProgramUniform3f(shaderProgram, GetAttribLocation(shaderProgram, name), vec[0], vec[1], vec[2]);
}

void ShaderSetMat4(GLuint shaderProgram, const char* name, mat4 matrix)
{
  glUniformMatrix4fv(GetAttribLocation(shaderProgram, name), 1, GL_FALSE, matrix[0]);
  //Newer OpenGL: glProgramUniformMatrix4fv(shaderProgram, GetAttribLocation(shaderProgram, name), 1, GL_FALSE, matrix[0]);
}

void ShaderSetTexture2D(GLuint shaderProgram, const char* name, GLuint texture, GLint slot)
{
  ShaderSetInt1(shaderProgram, name, slot);
  Texture2DBind(texture, slot);
}

void ShaderSetTextureArray(GLuint shaderProgram, const char* name, GLuint texture, GLint slot)
{
  ShaderSetInt1(shaderProgram, name, slot);
  TextureArrayBind(texture, slot);
}

void ShaderSetTextureSkybox(GLuint shaderProgram, const char* name, GLuint texture, GLint slot)
{
  ShaderSetInt1(shaderProgram, name, slot);
  TextureSkyboxBind(texture, slot);
}

void ShaderUse(GLuint shaderProgram)
{
  glUseProgram(shaderProgram);
}

static void ShaderFreeOne(GLuint* shaderProgram)
{
  if(*shaderProgram)
  {
    glDeleteProgram(*shaderProgram);
    *shaderProgram = 0;
  }
}

void ShaderFreeAll()
{
  ShaderFreeOne(&SHADER_BLOCK);
  ShaderFreeOne(&SHADER_LINE);
  ShaderFreeOne(&SHADER_SKYBOX);
  ShaderFreeOne(&SHADER_SUN);
  ShaderFreeOne(&SHADER_DEFERRED);
  ShaderFreeOne(&SHADER_DEFERRED2);
  ShaderFreeOne(&SHADER_SHADOW);
  ShaderFreeOne(&SHADER_PIP);
  ShaderFreeOne(&SHADER_HAND_ITEM);
}