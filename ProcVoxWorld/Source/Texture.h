#pragma once

#include "glad/glad.h"

//Global variables for access by other files:
extern GLuint TEXTURE_BLOCKS;
extern GLuint TEXTURE_SKYBOX_DAY;
extern GLuint TEXTURE_SKYBOX_EVENING;
extern GLuint TEXTURE_SKYBOX_NIGHT;
extern GLuint TEXTURE_SUN;
extern GLuint TEXTURE_MOON;

void TextureInitAll();

GLuint FramebufferColorTextureCreate(GLsizei width, GLsizei height);

GLuint FramebufferDepthTextureCreate(GLsizei width, GLsizei height);

void Texture2DBind(GLuint texture, GLint slot);

void TextureArrayBind(GLuint texture, GLint slot);

void TextureSkyboxBind(GLuint texture, GLint slot);

void TextureFreeAll();