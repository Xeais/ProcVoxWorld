#pragma once

#include "glad/glad.h"
#include "cglm/cglm.h"

//Global variables for access by other files:
extern GLuint SHADER_BLOCK;
extern GLuint SHADER_LINE;
extern GLuint SHADER_SKYBOX;
extern GLuint SHADER_SUN;
extern GLuint SHADER_DEFERRED;
extern GLuint SHADER_DEFERRED2;
extern GLuint SHADER_SHADOW;
extern GLuint SHADER_PIP;
extern GLuint SHADER_HAND_ITEM;

void ShaderInitAll();

void ShaderSetInt1(GLuint shader, const char* name, GLint value);

void ShaderSetFloat1(GLuint shader, const char* name, GLfloat value);

void ShaderSetFloat3(GLuint shader, const char* name, vec3 vec);

void ShaderSetMat4(GLuint shader, const char* name, mat4 matrix);

void ShaderSetTexture2D(GLuint shader, const char* name, GLuint texture, GLint slot);

void ShaderSetTextureArray(GLuint shader, const char* name, GLuint texture, GLint slot);

void ShaderSetTextureSkybox(GLuint shader, const char* name, GLuint texture, GLint slot);

void ShaderUse(GLuint shader);

void ShaderFreeAll();