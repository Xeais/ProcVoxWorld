#pragma once

#include "glad/glad.h"

typedef enum
{
  FB_TYPE_DEFAULT,
  FB_TYPE_TEXTURE,
  FB_TYPE_SHADOW_NEAR,
  FB_TYPE_SHADOW_FAR
} FbType;

typedef enum
{
  TEX_COLOR, //As usual, C-enumerations start with zero.
  TEX_UI,
  TEX_PASS_ONE
} FbTextureType;

typedef struct
{
  /* The default framebuffer is the framebuffer that OpenGL is created with. 
   * It is created along with the OpenGL-context. Like framebuffer objects (FBOs), 
   * the default framebuffer is a series of images. Unlike FBOs, 
   * one of these images usually represents what is actually drawn on some part of the screen. */
  GLuint defaultFb;

  GLuint quadVBO;
  GLuint quadVAO;

  GLuint GBufFBO;
  GLuint GBufTexColor;
  GLuint GBufTexColorUI;
  GLuint GBufTexColorPassOne;
  GLuint GBufTexDepth;

  GLuint GBufShadowNear;
  GLuint GBufShadowNearMap;
  int32_t nearShadowMapW;

  GLuint GBufShadowFar;
  GLuint GBufShadowFarMap;
  int32_t farShadowMapW;
} Framebuffers;

Framebuffers* FramebufferCreateAll(GLsizei windowW, GLsizei windowH);

void FramebufferRebuildAll(Framebuffers* fbs, GLsizei newWindowW, GLsizei newWindowH);

void FramebufferUse(Framebuffers* fbs, FbType type);

void FramebufferUseTexture(/* Newer OpenGL: Framebuffers* fbs, */ FbTextureType type);

//For newer OpenGL: GLuint GetFbByType(FbType fbType);

void FramebufferDestroyAll(Framebuffers* fbs);

/* ----- Inline -----
 * Some consider this bad style, but I believe it can be used without remorse as long as three rules are respected:
 *   1. Inline functions have to be small since when a header file is included, there will be a seperate copy for each inline function it contains. 
 *      This is the strongest reason against it: https://softwareengineering.stackexchange.com/a/56230.
 *   2. Small inline functions can speed up code execution speed, consequently use them when this is important! 
 *   3. Do not use them excessively! Use them only when the two rules above really apply.
 * 
 * These are simple guidelines that work in most cases, but if you find yourself in a situation where you need to squeeze the last ounce of performance 
 * out of some really limited hardware (this is C, after all), I'm afraid you'll need to investigate further: https://stackoverflow.com/a/54213706.
 * I'll close with the excellent conclusion from the Stack Overflow link right above: 
 * "There are no simple answers: You have to play with it to see what is best. Do not settle for simplistic answers like, "Never use inline functions" or 
 * "Always use inline functions" or "Use inline functions if and only if the function is less than N lines of code." These one-size-fits-all rules may be easy to write down, 
 * but they will produce sub-optimal results." */

/* Newer OpenGL:
 * //"color", alternative syntax: const GLfloat color[4]
 * static inline void ClearColor(FbType fbType, GLint drawbufferIndex, const GLfloat* color)
 * {
 *   glClearNamedFramebufferfv(GetFbByType(fbType), GL_COLOR, drawbufferIndex, color);
 * }
 *
 * static inline void ClearDepth(FbType fbType, const GLfloat depth)
 * {
 *   //void glClearNamedFramebufferfv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat* value);
 *   glClearNamedFramebufferfv(GetFbByType(fbType), GL_DEPTH, 0, &depth); //If "buffer" is "GL_DEPTH", "drawbuffer" must be zero.
 * } */