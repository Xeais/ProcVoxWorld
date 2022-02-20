#include "Texture.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "Utils.h"

#include "stb/stb_image.h"

GLuint TEXTURE_BLOCKS;
GLuint TEXTURE_SKYBOX_DAY;
GLuint TEXTURE_SKYBOX_EVENING;
GLuint TEXTURE_SKYBOX_NIGHT;
GLuint TEXTURE_SUN;
GLuint TEXTURE_MOON;

static void ExitIfNotLoadedOrWrongChannels(const char* path, uint8_t* data, int32_t channels, int32_t channelsRequired)
{
  if(data == NULL)
  {
    LogError("Texture \"%s\" could not be opened.", true, path);
    glfwTerminate();

    exit(EXIT_FAILURE);
  }
  else if(channels != channelsRequired)
  {
    LogError("For image \"%s\" were %d channels expected, but %d were detected.", true, path, channelsRequired, channels);
    glfwTerminate();

    exit(EXIT_FAILURE);
  }
}

static GLuint TextureInit(GLuint target)
{
  GLuint texture;

  glGenTextures(1, &texture);
  glBindTexture(target, texture);

  //Newer OpenGL: glCreateTextures(target, 1, &texture); //DSA (Direct State Access: https://www.khronos.org/opengl/wiki/Direct_State_Access)

  return texture;
}

//Only for newer OpenGL: If a cube map is passed, increment the argument for the last parameter "face" by one each time it is called.
static void TextureLoadFromFile(/* Newer OpenGL: GLuint texture, */ GLuint target, const char* path, int32_t desiredChannels /* Newer OpenGL:, uint32_t face */)
{
  /* Newer OpenGL:
   * bool isCubeMap = false;
   * if(target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && target < GL_TEXTURE_CUBE_MAP_POSITIVE_X + 6)
   *   isCubeMap = true; */

  int32_t width, height, channels;
  uint8_t* data = stbi_load(path, &width, &height, &channels, 0);

  ExitIfNotLoadedOrWrongChannels(path, data, channels, desiredChannels);

  GLuint format = desiredChannels == 4 ? GL_RGBA : GL_RGB;
  /* void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * data);
   * -> "glTexImage2D - OpenGL 4 Reference Pages": https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml */
  glTexImage2D(target, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);


  /* Newer OpenGL;
   * Since no texture data is actually provided, the value used for "internalformat" (glTextureStorage2D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height))
   * is irrelevant and may be considered to be any value that is legal for the chosen "internalformat" enumerant. 
   * -> "glTexStorage2D - OpenGL 4 Reference Pages": https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml 
   * Hint: "glTexImage2D()" and "glTextureStorage2D()" are like C's "malloc()" (https://www.cplusplus.com/reference/cstdlib/malloc/).
   * GLenum format = desiredChannels == 4 ? GL_RGBA8 : GL_RGB4;
   * if(face == 0) //Initialize only once as immutable textures are used.
   *   glTextureStorage2D(texture, 1, format, width, height);
   *
   * void glTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
   * format: Specifies the format of the pixel data. The following symbolic values are accepted: GL_RED, GL_RG, GL_RGB, GL_BGR, GL_RGBA, GL_BGRA, GL_DEPTH_COMPONENT and GL_STENCIL_INDEX. 
   * -> "glTextureSubImage2D - OpenGL 4 Reference Pages": https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexSubImage2D.xhtml
   * Hint: "glTextureSubImage2D()" is like "memcpy()" (https://www.cplusplus.com/reference/cstring/memcpy/); it only works for existing memory. 
   * format = desiredChannels == 4 ? GL_RGBA : GL_RGB;
   * if(!isCubeMap)
   *   glTextureSubImage2D(texture, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
   * else
   *   glTextureSubImage3D(texture, 0, 0, 0, face, width, height, 1, format, GL_UNSIGNED_BYTE, data);
   * void glTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
   * -> "glTexSubImage3D - OpenGL 4 Reference Pages": https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexSubImage3D.xhtml */

  stbi_image_free(data);
}

static GLuint Texture2DCreate(const char* path)
{
  GLuint texture = TextureInit(GL_TEXTURE_2D);
  TextureLoadFromFile(GL_TEXTURE_2D, path, 4);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  /* Newer OpenGL:
   * glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   * glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   * glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   * glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST); */

  return texture;
}

static GLuint TextureArrayCreate(const char* path)
{
  GLuint texture = TextureInit(GL_TEXTURE_2D_ARRAY);

  stbi_set_flip_vertically_on_load(1);
  int32_t width, height, channels;
  uint8_t* data = stbi_load(path, &width, &height, &channels, 0);

  ExitIfNotLoadedOrWrongChannels(path, data, channels, 4);

  int32_t atlasRowSize = width * channels;

  int32_t tiles = 256;
  int32_t tileWidth = 16;
  int32_t tileHeight = 16;
  int32_t tileRowSize = tileWidth * channels;

  /* void glTexImage3D(GLenum target,	GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void * data);
   * -> "glTexImage3D - OpenGL 4 Reference Pages": https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexImage3D.xhtml */
  glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, tileWidth, tileHeight, tiles, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

 /* Newer OpenGL:
  * void glTextureStorage3D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
  * -> "glTexStorage3D - OpenGL 4 Reference Pages": https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage3D.xhtml
  * glTextureStorage3D(texture, 1, GL_RGBA8, tileWidth, tileHeight, tiles); */

  //16 * 16 pixels, 4 bytes per pixel:
  uint8_t* tileData = (uint8_t*)OwnMalloc((uintmax_t)tileWidth * tileHeight * channels, false);

  if(tileData == NULL)
  {
    LogError("Variable \"tileData\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return EXIT_FAILURE;
  }

  for(uint32_t y = 0; y < 16; ++y)
  {
    for(uint32_t x = 0; x < 16; ++x)
    {
      //Extract tile data from the texture atlas.
      for(int32_t row = 0; row < tileHeight; ++row)
      {
        uint8_t* dst = tileData + row * tileRowSize;

        if(dst == NULL)
        {
          LogError("Variable \"dst\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

          return 0;
        }

        memcpy(dst, data + (x * tileRowSize) + atlasRowSize * (y * tileHeight + row), tileRowSize);
      }

      int32_t currTile = x + y * 16;

      /* void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels);
       * -> "glTexSubImage3D - OpenGL 4 Reference Pages": https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexSubImage3D.xhtml */
      glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, currTile, tileWidth, tileHeight, 1, GL_RGBA, GL_UNSIGNED_BYTE, tileData); 
 
      //Newer OpenGL: glTextureSubImage3D(texture, 0, 0, 0, currTile, tileWidth, tileHeight, 1, GL_RGBA, GL_UNSIGNED_BYTE, tileData);
    }
  }

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_REPEAT);

  /* Newer OpenGL:
   * glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
   * glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
   * glTextureParameteri(texture, GL_TEXTURE_WRAP_R, GL_REPEAT); */

  //If it is available, enable anisotropic filtering.
  if(GLAD_GL_EXT_texture_filter_anisotropic)
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, ANISOTROPIC_FILTER_LEVEL); //Newer OpenGL: glTextureParameteri(texture, GL_TEXTURE_MAX_ANISOTROPY_EXT, ANISOTROPIC_FILTER_LEVEL);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

  /* Newer OpenGL:
   * glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   * glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   *
   * glGenerateTextureMipmap(texture); */

  stbi_image_free(data);
  free(tileData);

  return texture;
}

static GLuint TextureSkyboxCreate(const char* paths[6])
{
  GLuint texture = TextureInit(GL_TEXTURE_CUBE_MAP);

  stbi_set_flip_vertically_on_load(0);

  for(int32_t i = 0; i < 6; ++i)
    TextureLoadFromFile(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, paths[i], 3); //Newer OpenGL: TextureLoadFromFile(texture, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, paths[i], 3, i);

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  /* Newer OpenGL:
   * glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   * glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   * glTextureParameteri(texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
   *
   * glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   * glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR); */

  return texture;
}

void TextureInitAll()
{
  TEXTURE_BLOCKS = TextureArrayCreate("Textures/Blocks.png");

  TEXTURE_SKYBOX_DAY = TextureSkyboxCreate((const char* [6])
  {
    "Textures/Skybox/Day/Right.png",
    "Textures/Skybox/Day/Left.png",
    "Textures/Skybox/Day/Top.png",
    "Textures/Skybox/Day/Bottom.png",
    "Textures/Skybox/Day/Front.png",
    "Textures/Skybox/Day/Back.png"
  });

  TEXTURE_SKYBOX_EVENING = TextureSkyboxCreate((const char* [6])
  {
    "Textures/Skybox/Evening/Right.png",
    "Textures/Skybox/Evening/Left.png",
    "Textures/Skybox/Evening/Top.png",
    "Textures/Skybox/Evening/Bottom.png",
    "Textures/Skybox/Evening/Front.png",
    "Textures/Skybox/Evening/Back.png"
  });

  TEXTURE_SKYBOX_NIGHT = TextureSkyboxCreate((const char* [6])
  {
    "Textures/Skybox/Night/Right.png",
    "Textures/Skybox/Night/Left.png",
    "Textures/Skybox/Night/Top.png",
    "Textures/Skybox/Night/Bottom.png",
    "Textures/Skybox/Night/Front.png",
    "Textures/Skybox/Night/Back.png"
  });

  TEXTURE_SUN = Texture2DCreate("Textures/Sun.png");
  TEXTURE_MOON = Texture2DCreate("Textures/Moon.png");
}

GLuint FramebufferColorTextureCreate(GLsizei width, GLsizei height)
{
  GLuint texture = TextureInit(GL_TEXTURE_2D);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  /* Newer OpenGL:
   * glTextureStorage2D(texture, 1, GL_RGBA8, width, height);
   *
   * glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   * glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   * glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   " glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR); */

  return texture;
}

GLuint FramebufferDepthTextureCreate(GLsizei width, GLsizei height)
{
  GLuint texture = TextureInit(GL_TEXTURE_2D);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

  /* Newer OpenGL: 
   * glTextureStorage2D(texture, 1, GL_DEPTH_COMPONENT32F, width, height);
   *
   * glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   * glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   * glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   * glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR); */

  return texture;
}

void Texture2DBind(GLuint texture, GLint slot)
{
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, texture);

  //Newer OpenGL: glBindTextureUnit(GL_TEXTURE0 + slot, texture);
}

void TextureArrayBind(GLuint texture, GLint slot)
{
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture);

  //Newer OpenGL: glBindTextureUnit(GL_TEXTURE0 + slot, texture);
}

void TextureSkyboxBind(GLuint texture, GLint slot)
{
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

  //Newer OpenGL: glBindTextureUnit(GL_TEXTURE0 + slot, texture);
}

static void TextureFreeOne(GLuint* texture)
{
  if(*texture)
  {
    glDeleteTextures(1, texture);
    *texture = 0;
  }
}

void TextureFreeAll()
{
  TextureFreeOne(&TEXTURE_BLOCKS);
  TextureFreeOne(&TEXTURE_SKYBOX_DAY);
  TextureFreeOne(&TEXTURE_SKYBOX_EVENING);
  TextureFreeOne(&TEXTURE_SKYBOX_NIGHT);
  TextureFreeOne(&TEXTURE_SUN);
  TextureFreeOne(&TEXTURE_MOON);
}