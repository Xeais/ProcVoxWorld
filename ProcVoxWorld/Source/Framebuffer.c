#include "Framebuffer.h"

#include "Window.h"
#include "Texture.h"

static void FBsCallback(void* thisObject, GLsizei newWidth, GLsizei newHeight)
{
  Framebuffers* fbs = (Framebuffers*)thisObject;
  FramebufferRebuildAll(fbs, newWidth, newHeight);
}

static const char* GetSpecificFbErrorMsg(GLenum fbStatus)
{
  switch(fbStatus)
  {
    case GL_FRAMEBUFFER_UNDEFINED:
      return "The specified framebuffer is the default read or draw framebuffer, but the default framebuffer does not exist.";
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      return "One or more of the framebuffer attachment points are framebuffer incomplete.";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      return "The framebuffer does not have at least one image attached to it.";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      return "The value of \"GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE\" is \"GL_NONE\" for any color attachment point(s) named by \"GL_DRAW_BUFFERi\".";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      return "\"GL_READ_BUFFER\" is not \"GL_NONE\" and the value of \"GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE\" is \"GL_NONE\" for the color attachment point named by \"GL_READ_BUFFER\".";
    case GL_FRAMEBUFFER_UNSUPPORTED:
      return "The combination of internal formats of the attached images violates an implementation-dependent set of restrictions.";
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
      return "The value of \"GL_RENDERBUFFER_SAMPLES\" is not the same for all attached renderbuffers; if the value of \"GL_TEXTURE_SAMPLES\" is the not same for all attached textures "
        "or, if the attached images are a mix of renderbuffersand textures, the value of \"GL_RENDERBUFFER_SAMPLES\" does not match the value of \"GL_TEXTURE_SAMPLES\".\n"
        "Another possibility: The value of \"GL_TEXTURE_FIXED_SAMPLE_LOCATIONS\" is not the same for all attached textures or, if the attached images are a mix of renderbuffersand textures, "
        "the value of \"GL_TEXTURE_FIXED_SAMPLE_LOCATIONS\" is not \"GL_TRUE\" for all attached textures.";
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
      return "At least one framebuffer attachment is layered and any populated attachment is not layered or if all populated color attachments are not from textures of the same target.";
    default:
      return "Unspecified error emerged.";
  }
}

//Create the geometry buffer (G-Buffer).
static void CreateGBuf(Framebuffers* fbs, GLsizei windowW, GLsizei windowH)
{
  fbs->GBufFBO = OpenGLCreateFBO();

  fbs->GBufTexColor = FramebufferColorTextureCreate(windowW, windowH);
  fbs->GBufTexColorUI = FramebufferColorTextureCreate(windowW, windowH);
  fbs->GBufTexColorPassOne = FramebufferColorTextureCreate(windowW, windowH);
  fbs->GBufTexDepth = FramebufferDepthTextureCreate(windowW, windowH);

  /* Newer OpenGL:
   * GBufFBO
   * -------
   * 
   * - GL_COLOR_ATTACHMENT0 -> GBufTexColor
   * - GL_COLOR_ATTACHMENT1 -> GBufTexColorUI
   * - GL_COLOR_ATTACHMENT2 -> GBufTexColorPassOne
   * 
   * This attributions are especially important for "FramebufferUseTexture()" (further down in this file). 
   * 
   * void glNamedFramebufferTexture(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
   * -> "glFramebufferTexture - OpenGL 4 Reference Pages": https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glFramebufferTexture.xhtml 
   * glNamedFramebufferTexture(fbs->GBufFBO, GL_COLOR_ATTACHMENT0, fbs->GBufTexColor, 0);
   * glNamedFramebufferTexture(fbs->GBufFBO, GL_COLOR_ATTACHMENT1, fbs->GBufTexColorUI, 0);
   * glNamedFramebufferTexture(fbs->GBufFBO, GL_COLOR_ATTACHMENT2, fbs->GBufTexColorPassOne, 0);
   * glNamedFramebufferTexture(fbs->GBufFBO, GL_DEPTH_ATTACHMENT, fbs->GBufTexDepth, 0);
   *
   * GLenum colorAttachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
   * glNamedFramebufferDrawBuffers(fbs->GBufFBO, ARRAY_SIZE(colorAttachments), colorAttachments);
   *
   * //-> "glCheckFramebufferStatus - OpenGL 4 Reference Pages": https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glCheckFramebufferStatus.xhtml
   * if(glCheckNamedFramebufferStatus(fbs->GBufFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) */

   //void glFramebufferTexture2D(GLenum target,	GLenum attachment, GLenum textarget, GLuint texture, GLint level); 
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbs->GBufTexColor, 0);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fbs->GBufTexColorUI, 0);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, fbs->GBufTexColorPassOne, 0);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbs->GBufTexDepth, 0);

   GLenum fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
   if(fbStatus != GL_FRAMEBUFFER_COMPLETE)
     LogError("The G-Buffer framebuffer is incomplete!\nMore specific: %s", true, GetSpecificFbErrorMsg(fbStatus));
}

static void DeleteGBuf(Framebuffers* fbs)
{
  glDeleteTextures(4, (GLuint[])
  {
    fbs->GBufTexColor,
    fbs->GBufTexColorUI,
    fbs->GBufTexColorPassOne, 
    fbs->GBufTexDepth
  });
  glDeleteFramebuffers(1, &fbs->GBufFBO);
}

Framebuffers* FramebufferCreateAll(GLsizei windowW, GLsizei windowH)
{
  Framebuffers* fbs = (Framebuffers*)OwnMalloc(sizeof(Framebuffers), false);

  if(fbs == NULL)
  {
    LogError("Variable \"fbs\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return NULL;
  }

  RegisterFramebufferSizeChangeCallback(fbs, FBsCallback);

  //Zero stands for the default-created FBO which is bound to the GLFW-window.
  fbs->defaultFb = 0;

  static float screenVertices[] = 
  {
    //      pos            texCoord  
    -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
     1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,

    -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,
     1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,   1.0f, 1.0f
  };

  fbs->quadVAO = OpenGLCreateVAO();
  fbs->quadVBO = OpenGLCreateVBO(screenVertices, sizeof(screenVertices));

  OpenGL_VBOLayout(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
  OpenGL_VBOLayout(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));

  /* Newer OpenGL:
   * OpenGL_VBOLayout(fbs->quadVAO, fbs->quadVBO, 0, 0, 3, GL_FLOAT, GL_FALSE, 0, 5 * sizeof(float));
   * OpenGL_VBOLayout(fbs->quadVAO, fbs->quadVBO, 1, 0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 5 * sizeof(float)); */

  CreateGBuf(fbs, windowW, windowH);

  fbs->GBufShadowNear = OpenGLCreateFBO();

  fbs->nearShadowMapW = 2048;
  fbs->GBufShadowNearMap = FramebufferDepthTextureCreate(fbs->nearShadowMapW, fbs->nearShadowMapW);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

  /* Newer OpenGL:
   * glTextureParameteri(fbs->GBufShadowNearMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
   * glTextureParameteri(fbs->GBufShadowNearMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
   * glTextureParameteri(fbs->GBufShadowNearMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   * glTextureParameteri(fbs->GBufShadowNearMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   *
   * glTextureParameterfv(fbs->GBufShadowNearMap, GL_TEXTURE_BORDER_COLOR, borderColor);
   * glTextureParameteri(fbs->GBufShadowNearMap, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
   * glTextureParameteri(fbs->GBufShadowNearMap, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE); */

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbs->GBufShadowNearMap, 0);

  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  /* Newer OpenGL:
   * glNamedFramebufferTexture(fbs->GBufShadowNear, GL_DEPTH_ATTACHMENT, fbs->GBufShadowNearMap, 0);
   *
   * glNamedFramebufferDrawBuffer(fbs->GBufShadowNear, GL_NONE);
   * glNamedFramebufferReadBuffer(fbs->GBufShadowNear, GL_NONE);
   *
   * fbStatus = glCheckNamedFramebufferStatus(fbs->GBufShadowNear, GL_FRAMEBUFFER); */

  GLenum fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(fbStatus != GL_FRAMEBUFFER_COMPLETE)
    LogError("The near shadow framebuffer is incomplete!\nMore specific: %s", true, GetSpecificFbErrorMsg(fbStatus));

  //Make the default framebuffer active (disable off-screen rendering).
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
 
  fbs->GBufShadowFar = OpenGLCreateFBO();

  fbs->farShadowMapW = 2048;
  fbs->GBufShadowFarMap = FramebufferDepthTextureCreate(fbs->farShadowMapW, fbs->farShadowMapW);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

  /* Newer OpenGL:
   * glTextureParameteri(fbs->GBufShadowFarMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
   * glTextureParameteri(fbs->GBufShadowFarMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
   * glTextureParameteri(fbs->GBufShadowFarMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   * glTextureParameteri(fbs->GBufShadowFarMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   * glTextureParameterfv(fbs->GBufShadowFarMap, GL_TEXTURE_BORDER_COLOR, borderColor);
   * glTextureParameteri(fbs->GBufShadowFarMap, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
   * glTextureParameteri(fbs->GBufShadowFarMap, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE); */

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbs->GBufShadowFarMap, 0);

  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  /* Newer OpenGL: 
   * glNamedFramebufferTexture(fbs->GBufShadowFar, GL_DEPTH_ATTACHMENT, fbs->GBufShadowFarMap, 0);
   *
   * glNamedFramebufferDrawBuffer(fbs->GBufShadowFar, GL_NONE);
   * glNamedFramebufferReadBuffer(fbs->GBufShadowFar, GL_NONE);
   *
   * fbStatus = glCheckNamedFramebufferStatus(fbs->GBufShadowFar, GL_FRAMEBUFFER); */

  fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(fbStatus != GL_FRAMEBUFFER_COMPLETE)
    LogError("The far shadow framebuffer is incomplete!\nMore specific: %s", true, GetSpecificFbErrorMsg(fbStatus));

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return fbs;
}

void FramebufferRebuildAll(Framebuffers* fbs, GLsizei newWindowW, GLsizei newWindowH)
{
  DeleteGBuf(fbs);
  CreateGBuf(fbs, newWindowW, newWindowH);
}

void FramebufferUse(Framebuffers* fbs, FbType type)
{
  switch(type)
  {
    case FB_TYPE_DEFAULT:
      glBindFramebuffer(GL_FRAMEBUFFER, fbs->defaultFb);
      break;
    case FB_TYPE_TEXTURE:
      glBindFramebuffer(GL_FRAMEBUFFER, fbs->GBufFBO);
      break;
    case FB_TYPE_SHADOW_NEAR:
      glBindFramebuffer(GL_FRAMEBUFFER, fbs->GBufShadowNear);
      break;
    case FB_TYPE_SHADOW_FAR:
      glBindFramebuffer(GL_FRAMEBUFFER, fbs->GBufShadowFar);
      break;
    default:
      LogError("The type of framebuffer \"%d\" provided is not taken care of. This really shouldn't happen!", true, type);
      break;
  }
}

void FramebufferUseTexture(/* Newer OpenGL: Framebuffers* fbs, */FbTextureType type)
{
  switch(type)
  {
    case TEX_COLOR:
      glDrawBuffer(GL_COLOR_ATTACHMENT0);
      glClearBufferfv(GL_COLOR, 0, (const GLfloat[]){1.0f, 0.0f, 0.0f, 1.0f});

      /* Newer OpenGL:
       * glNamedFramebufferDrawBuffer(fbs->GBufFBO, GL_COLOR_ATTACHMENT0);
       * glClearNamedFramebufferfv(fbs->GBufFBO, GL_COLOR, 0, (const GLfloat[]){1.0f, 0.0f, 0.0f, 1.0f}); */
      break;
    case TEX_UI:
      glDrawBuffer(GL_COLOR_ATTACHMENT1);
      glClearBufferfv(GL_COLOR, 0, (const GLfloat[]){0.0f, 1.0f, 0.0f, 0.0f});

      /* Newer OpenGL:
       * glNamedFramebufferDrawBuffer(fbs->GBufFBO, GL_COLOR_ATTACHMENT1);
       * glClearNamedFramebufferfv(fbs->GBufFBO, GL_COLOR, 0, (const GLfloat[]){0.0f, 1.0f, 0.0f, 0.0f}); */
      break;
    case TEX_PASS_ONE:
      glDrawBuffer(GL_COLOR_ATTACHMENT2);
      glClearBufferfv(GL_COLOR, 0, (const GLfloat[]){1.0f, 0.0f, 1.0f, 1.0f});

      /* Newer OpenGL:
       * glNamedFramebufferDrawBuffer(fbs->GBufFBO, GL_COLOR_ATTACHMENT2);
       * glClearNamedFramebufferfv(fbs->GBufFBO, GL_COLOR, 0, (const GLfloat[]){1.0f, 0.0f, 1.0f, 1.0f}); */
      break;
    default:
      LogError("The provided framebuffer texture type \"%d\" is not taken care of. This really shouldn't happen!", true, type);
      break;
  }
}

/* For newer OpenGL:
 * GLuint GetFbByType(FbType fbType)
 * {
 *   switch(fbType)
 *   {
 *     case FB_TYPE_DEFAULT:
 *       return WND->fb->defaultFb;
 *     case FB_TYPE_TEXTURE:
 *       return WND->fb->GBufFBO;
 *     case FB_TYPE_SHADOW_NEAR:
 *       return WND->fb->GBufShadowNear;
 *     case FB_TYPE_SHADOW_FAR:
 *       return WND->fb->GBufShadowFar;
 *     default:
 *       LogWarning("The provided framebuffer type \"%d\" does not exist, hence, the default framebuffer has been returned.", true, fbType);
 *
 *       return WND->fb->defaultFb;
 *   }
 * } */

void FramebufferDestroyAll(Framebuffers* fbs)
{
  DeleteGBuf(fbs);
  glDeleteBuffers(1, &fbs->quadVBO);
  glDeleteVertexArrays(1, &fbs->quadVAO);

  free(fbs);
}