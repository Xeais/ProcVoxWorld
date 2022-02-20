#include "Database.h"
#include "TimeMeasurement.h"

#include "Window.h"
#include "Shader.h"
#include "Texture.h"

#include "Camera/CameraController.h"
#include "Player/PlayerController.h"

#include <time.h>
#if PLATFORM_POSIX
#include <unistd.h>
#endif

//Tell the Nvidia or AMD driver to prefer the discrete GPU rather than the CPU integrated one.
extern __declspec(dllexport) DWORD NvOptimusEnablement = 1;
extern __declspec(dllexport) int32_t AmdPowerXpressRequestHighPerformance = 1;

extern bool STDOUT_SUPPORTS_COLORS = false;
extern bool STDERR_SUPPORTS_COLORS = false;
//Global for this file:
static mat4 nearShadowMapMat;
static mat4 farShadowMapMat;

static float nearShadowMapSize;
static float farShadowMapSize;

typedef enum
{
  NEAR_PLANE_DEFAULT,
  NEAR_PLANE_EXTENDED
} NearPlaneType;

static void PressEnterToContinue(void)
{
  LogInfo("Press the enter/return key to continue ...", true);

  //The ASCII-code of the enter key is ten in decimal or "0x0A" in hexadecimal.
  if(getchar() == 10) //"getchar()" is easy, cross-platform and standardized.
    return;
}

//The signature is predefined - after all, it's a callback function!
static void OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param)
{
  //Suppress non-significant warnings. (The not unjustly best-known online OpenGL tutorial does the same -https://learnopengl.com/In-Practice/Debugging https://learnopengl.com/In-Practice/Debugging.)
  switch(id)
  {
    case 131185: //-> OpenGL debug context warning - "Will use VIDEO memory as the source for buffer objection": https://stackoverflow.com/a/62249363
      return;
    case 131218: /* -> "Program/shader state performance warning: Vertex shader in program X is being recompiled based on GL state.":
                  *    https://computergraphics.stackexchange.com/a/9098 - yes, it only appears at the beginning! */
      return;
  }

  char intSource[25] = "";
  char intType[25] = "";
  char intSeverity[15] = "";

  switch(source)
  {
    case GL_DEBUG_SOURCE_API:
      strcpy_s(intSource, ARRAY_SIZE(intSource), "API");
      break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      strcpy_s(intSource, ARRAY_SIZE(intSource), "The window system");
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      strcpy_s(intSource, ARRAY_SIZE(intSource), "The shader compiler");
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      strcpy_s(intSource, ARRAY_SIZE(intSource), "Third party");
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
      strcpy_s(intSource, ARRAY_SIZE(intSource), "The application");
      break;
    case GL_DEBUG_SOURCE_OTHER:
    default:
      strcpy_s(intSource, ARRAY_SIZE(intSource), "Other");
      break;
  }

  switch(type)
  {
    case GL_DEBUG_TYPE_ERROR:
      strcpy_s(intType, ARRAY_SIZE(intType), "an error");
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      strcpy_s(intType, ARRAY_SIZE(intType), "deprecated behavior");
      break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      strcpy_s(intType, ARRAY_SIZE(intType), "undefined behavior");
      break;
    case GL_DEBUG_TYPE_PORTABILITY:
      strcpy_s(intType, ARRAY_SIZE(intType), "a portability issue");
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
      strcpy_s(intType, ARRAY_SIZE(intType), "a performance problem");
      break;
    case GL_DEBUG_TYPE_MARKER:
      strcpy_s(intType, ARRAY_SIZE(intType), "a marker");
      break;
    case GL_DEBUG_TYPE_OTHER:
    default:
      strcpy_s(intType, ARRAY_SIZE(intType), "\"GL_DEBUG_TYPE_OTHER\"");
      break;
  }

  switch(severity)
  {
    case GL_DEBUG_SEVERITY_HIGH:
      strcpy_s(intSeverity, ARRAY_SIZE(intSeverity), "high");
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      strcpy_s(intSeverity, ARRAY_SIZE(intSeverity), "medium");
      break;
    case GL_DEBUG_SEVERITY_LOW:
      strcpy_s(intSeverity, ARRAY_SIZE(intSeverity), "low");
      break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
    default:
      strcpy_s(intSeverity, ARRAY_SIZE(intSeverity), "notification");
      break;
  }

  if(strcmp(intType, "an error") == 0 /* Strings are identical. */ || strcmp(intType, "undefined behavior") == 0)
    LogError("%s reports %s (severity: %s%s%s)\n[%sId = %u%s]: %s (length: %d chars)", true, intSource, intType, BOLD, intSeverity, NOBOLD, LINE, id, NOLINE, message, length);
  else if(strcmp(intType, "deprecated behavior") == 0 || strcmp(intType, "a portability issue") == 0 || strcmp(intType, "a performance problem") == 0)
    LogWarning("%s reports %s (severity: %s%s%s)\n[%sId = %u%s]: %s (length: %d chars)", true, intSource, intType, BOLD, intSeverity, NOBOLD, LINE, id, NOLINE, message, length);
  else //Marker and "GL_DEBUG_TYPE_OTHER"
    LogInfo("%s reports %s (severity: %s%s%s)\n[%sId = %u%s]: %s (length: %d chars)", true, intSource, intType, BOLD, intSeverity, NOBOLD, LINE, id, NOLINE, message, length);

  user_param; //A reference to resolve "C4100".
}

static float GetCurrentDOFDepth(float dt)
{
  static float currDepth = 1.0f;

  //Read depth in the center of the screen and gradually move towards it.

  float desiredDepth;
  glReadPixels(WND->width / 2, WND->height / 2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &desiredDepth); /* A synchronous readback is the fastest way to make the pixel data available as quickly as possible.
                                                                                                     * More information on the topic: https://stackoverflow.com/a/25127895 */

  /* Newer OpenGL:
   * int32_t bufferWidth = (WND->width + 7) & ~7; //Round width up to a multiple of eight to avoid alignment problems.
   * GLsizei bufferSize = bufferWidth * WND->height * 3;
   * glReadnPixels(WND->width / 2, WND->height / 2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, bufferSize, &desiredDepth); */

  currDepth = glm_lerp(currDepth, desiredDepth, dt * DOF_SPEED);

  return currDepth;
}

static void Update(PlayerController* pC, CameraController* cC, float dt)
{
  PlayerControllerDoControl(pC);
  PlayerUpdate(pC->player);
  CameraControllerDoControl(cC);
  MapUpdate(cC->camera);

  dt; //A reference to resolve "C4100".
}

static void GenShadowMapMat(mat4 res, Camera* cam, float sizeBlocks, NearPlaneType nearChoice)
{
  vec3 lightDir;
  MapGetLightDir(lightDir);

  vec3 lightPos;
  glm_vec3_copy(cam->pos, lightPos);

  //Mitigate shadow edge flickering:
  const float discreteStep = BLOCK_SIZE / 2.0f;
  for(uint32_t i = 0; i < 3; ++i)
    lightPos[i] = roundf(lightPos[i] / discreteStep) * discreteStep;

  mat4 lightViewMat;
  glm_look(lightPos, lightDir, cam->up, lightViewMat);

  float orthoRight = sizeBlocks / 2.0f * BLOCK_SIZE;
  float orthoLeft = -orthoRight;
  float orthoTop = sizeBlocks / 2.0f * BLOCK_SIZE;
  float orthoBottom = -orthoTop;
  float orthoFar = sizeBlocks / 2.0f * BLOCK_SIZE;
  float orthoNear = 0.0f;

  switch(nearChoice)
  {
    case NEAR_PLANE_DEFAULT:
      orthoNear = -orthoFar;
      break;
    case NEAR_PLANE_EXTENDED:
      orthoNear = -(CHUNK_RENDER_RADIUS + 3) * CHUNK_WIDTH * BLOCK_SIZE;
      break;
  }

  mat4 lightProjMat;
  glm_ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, orthoNear, orthoFar, lightProjMat);
  glm_mat4_mul(lightProjMat, lightViewMat, res);
}

static void GenShadowMapPlanes(vec4 res[6], Camera* cam, float sizeBlocks)
{
  mat4 lightMat;
  GenShadowMapMat(lightMat, cam, sizeBlocks, NEAR_PLANE_EXTENDED);
  glm_frustum_planes(lightMat, res);
}

static void RenderShadowMapPlanes(mat4 lightMat, vec4 frustumPlanes[6], int32_t shadowMapTexWidth, FbType fbType, float polygonOffset)
{
  ShaderUse(SHADER_SHADOW);

  ShaderSetMat4(SHADER_SHADOW, "MVPMatrix", lightMat);
  ShaderSetTextureArray(SHADER_SHADOW, "uBlocksTexture", TEXTURE_BLOCKS, 0);

  FramebufferUse(WND->fb, fbType);

  glClear(GL_DEPTH_BUFFER_BIT);
  //Newer OpenGL: ClearDepth(fbType, CLEAR_DEPTH);

  glViewport(0, 0, shadowMapTexWidth, shadowMapTexWidth);
  glPolygonOffset(polygonOffset, polygonOffset);

  MapRenderChunksRaw(frustumPlanes);
}

static void RenderAllShadowMaps(Camera* cam)
{
  nearShadowMapSize = 50.0f;
  farShadowMapSize = (CHUNK_RENDER_RADIUS + 3.0f) * CHUNK_WIDTH;

  GenShadowMapMat(nearShadowMapMat, cam, nearShadowMapSize, NEAR_PLANE_DEFAULT);
  GenShadowMapMat(farShadowMapMat, cam, farShadowMapSize, NEAR_PLANE_DEFAULT);

  vec4 nearPlanes[6], farPlanes[6];
  GenShadowMapPlanes(nearPlanes, cam, nearShadowMapSize);
  GenShadowMapPlanes(farPlanes, cam, farShadowMapSize);

  glEnable(GL_DEPTH_CLAMP);
  glEnable(GL_POLYGON_OFFSET_FILL);

  RenderShadowMapPlanes(nearShadowMapMat, nearPlanes, WND->fb->nearShadowMapW, FB_TYPE_SHADOW_NEAR, 4.0f);
  RenderShadowMapPlanes(farShadowMapMat, farPlanes, WND->fb->farShadowMapW, FB_TYPE_SHADOW_FAR, 8.0f);

  glDisable(GL_POLYGON_OFFSET_FILL);
  glDisable(GL_DEPTH_CLAMP);
}

static void RenderGame(Player* p, Camera* cam)
{
  glViewport(0, 0, WND->width, WND->height);

  FramebufferUse(WND->fb, FB_TYPE_TEXTURE);

  glClear(GL_DEPTH_BUFFER_BIT);
  //Newer OpenGL: ClearDepth(FB_TYPE_TEXTURE, CLEAR_DEPTH);

  FramebufferUseTexture(TEX_COLOR);
  {
    MapRenderSky(cam);
    MapRenderSunMoon(cam);
    MapRenderChunks(cam, nearShadowMapMat, farShadowMapMat);
  }

  FramebufferUseTexture(TEX_UI);
  {
    if(p->pointingAtBlock)
      UIRenderBlockWireframe(p, cam);

    UIRenderCrosshairs();
    //PlayerRenderItem(p);
  }
}

//Apply depth of field and render this to texture.
static void RenderFirstPass(float dt)
{
  ShaderUse(SHADER_DEFERRED);

  ShaderSetTexture2D(SHADER_DEFERRED, "textureColor", WND->fb->GBufTexColor, 0);
  ShaderSetTexture2D(SHADER_DEFERRED, "textureDepth", WND->fb->GBufTexDepth, 1);

  ShaderSetInt1(SHADER_DEFERRED, "uDOFEnabled", DOF_ENABLED);
  if(DOF_ENABLED)
  {
    ShaderSetInt1(SHADER_DEFERRED, "uDOFSmooth", DOF_SMOOTH);
    ShaderSetFloat1(SHADER_DEFERRED, "uMaxBlur", DOF_MAX_BLUR);
    ShaderSetFloat1(SHADER_DEFERRED, "uDOFAperture", DOF_APERTURE);
    ShaderSetFloat1(SHADER_DEFERRED, "uAspectRatio", (float)WND->width / WND->height);

    float currDepth = DOF_SMOOTH ? GetCurrentDOFDepth(dt) : 0.0f;
    ShaderSetFloat1(SHADER_DEFERRED, "uDepth", currDepth);
  }

  FramebufferUseTexture(TEX_PASS_ONE);
  glBindVertexArray(WND->fb->quadVAO);
  glDepthFunc(GL_ALWAYS);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

//Apply motion blur, gamma correction, saturation and finally, render it all to screen.
static void RenderSecondPass(Player* p, Camera* cam, float dt)
{
  ShaderUse(SHADER_DEFERRED2);

  ShaderSetTexture2D(SHADER_DEFERRED2, "textureColor", WND->fb->GBufTexColorPassOne, 0);
  ShaderSetTexture2D(SHADER_DEFERRED2, "textureUI", WND->fb->GBufTexColorUI, 1);
  ShaderSetTexture2D(SHADER_DEFERRED2, "textureDepth", WND->fb->GBufTexDepth, 2);

  ShaderSetInt1(SHADER_DEFERRED2, "uMotionBlurEnabled", MOTION_BLUR_ENABLED);
  if(MOTION_BLUR_ENABLED)
  {
    mat4 matrix;
    glm_mat4_inv(cam->projMatrix, matrix);
    ShaderSetMat4(SHADER_DEFERRED2, "uProjectionInvMat", matrix);

    glm_mat4_inv(cam->viewMatrix, matrix);
    ShaderSetMat4(SHADER_DEFERRED2, "uViewInvMat", matrix);

    glm_mat4_copy(cam->prevViewMatrix, matrix);
    ShaderSetMat4(SHADER_DEFERRED2, "uPrevViewMat", matrix);

    glm_mat4_copy(cam->projMatrix, matrix);
    ShaderSetMat4(SHADER_DEFERRED2, "uProjectionMat", matrix);

    ShaderSetFloat3(SHADER_DEFERRED2, "uCamPos", cam->pos);
    ShaderSetFloat3(SHADER_DEFERRED2, "uPrevCamPos", cam->prevPos);

    ShaderSetFloat1(SHADER_DEFERRED2, "uStrength", MOTION_BLUR_STRENGTH);
    ShaderSetInt1(SHADER_DEFERRED2, "uSamples", MOTION_BLUR_SAMPLES);
    ShaderSetFloat1(SHADER_DEFERRED2, "uDt", dt);
  }

  ShaderSetFloat1(SHADER_DEFERRED2, "uGamma", GAMMA);
  ShaderSetFloat1(SHADER_DEFERRED2, "uSaturation", SATURATION);

  FramebufferUse(WND->fb, FB_TYPE_DEFAULT);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* Newer OpenGL:
   * ClearColor(FB_TYPE_DEFAULT, 0, DEFAULT_CLEAR_COLOR);
   * ClearDepth(FB_TYPE_DEFAULT, CLEAR_DEPTH); */

  glBindVertexArray(WND->fb->quadVAO);
  glDepthFunc(GL_ALWAYS);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  //=============== Debug Picture in Picture View for Shadow Maps ===============

  if(WND->showPip)
  {
    ShaderUse(SHADER_PIP);

    ShaderSetTexture2D(SHADER_PIP, "uTexture", WND->fb->GBufShadowNearMap, 0);
    int32_t width = 325;
    int32_t height = 325;
    glViewport(10, WND->height - height - 10, width, height);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    ShaderSetTexture2D(SHADER_PIP, "uTexture", WND->fb->GBufShadowFarMap, 0);
    glViewport(WND->width - width - 10, WND->height - height - 10, width, height);
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  p; //A reference to resolve "C4100".
}

static void Render(Player* p, Camera* cam, float dt)
{
  RenderAllShadowMaps(cam);
  RenderGame(p, cam);
  RenderFirstPass(dt);
  RenderSecondPass(p, cam, dt);
}

/* ================== *
 * |  --  main  --  | *
 * ================== *
 *
 * The argument vector "argVec" is a tokenized representation of the command line that the program was invoked with. */
int32_t main(int32_t argCount, const char* argVec[])
{
  //Registers the function given as argument to be called on normal program termination (via "exit()" or returning from the main function).
  atexit(PressEnterToContinue);

  //Initialize random seed:
  srand((uint32_t)time(NULL));

#ifdef PLATFORM_WINDOWS
  HANDLE stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  if(stdOutHandle == INVALID_HANDLE_VALUE)
    LogWarning("The standard output handle could not be retrieved.", true); //Only warnings (no errors with abort), for it is not really severe if the CLI is without colors.
  else
  {
    DWORD conModeOut;
    if(!GetConsoleMode(stdOutHandle, &conModeOut))
      LogWarning("The console mode for the standard output (stdout) could not be fetched.", true);
    else
    {
      STDOUT_SUPPORTS_COLORS = true;
      if(conModeOut & ENABLE_PROCESSED_OUTPUT && conModeOut & ENABLE_VIRTUAL_TERMINAL_PROCESSING) //-> C Operator Precedence: https://en.cppreference.com/w/c/language/operator_precedence
        LogInfo("\"ENABLE_PROCESSED_OUTPUT\" and \"ENABLE_VIRTUAL_TERMINAL_PROCESSING\" are already active, allowing colored output for the standard output (stdout).\n", false);
      else
      {
        conModeOut |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        if(!SetConsoleMode(stdOutHandle, conModeOut))
        {
          LogWarning("The console mode for the standard output (stdout) could not be set.", true);
          STDOUT_SUPPORTS_COLORS = false;
        }
        else
          LogSuccess("\"ENABLE_PROCESSED_OUTPUT\" and \"ENABLE_VIRTUAL_TERMINAL_PROCESSING\" were activated, allowing colored output for the standard output (stdout).\n", false);
      }

      if(STDOUT_SUPPORTS_COLORS)
      {
        LogInfo("Color meaning: Information |", false);
        LogSuccess("| Success", true);
      }  
    }
  }

  HANDLE stdErrHandle = GetStdHandle(STD_ERROR_HANDLE);
  if(stdErrHandle == INVALID_HANDLE_VALUE)
    LogWarning("The standard error handle could not be retrieved.", true);
  else
  {
    DWORD conModeErr;
    if(!GetConsoleMode(stdErrHandle, &conModeErr))
      LogWarning("The console mode for standard error (stderr) could not be fetched.", true);
    else
    {
      STDERR_SUPPORTS_COLORS = true;
      if(conModeErr & ENABLE_PROCESSED_OUTPUT && conModeErr & ENABLE_VIRTUAL_TERMINAL_PROCESSING)
        LogInfo("\"ENABLE_PROCESSED_OUTPUT\" and \"ENABLE_VIRTUAL_TERMINAL_PROCESSING\" are already active, allowing colored output for standard error (stderr).\n", false);
      else
      {
        conModeErr |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        if(!SetConsoleMode(stdErrHandle, conModeErr))
        {
          LogWarning("The console mode for standard error (stderr) could not be set.", true);
          STDERR_SUPPORTS_COLORS = false;
        }
        else
          LogSuccess("\"ENABLE_PROCESSED_OUTPUT\" and \"ENABLE_VIRTUAL_TERMINAL_PROCESSING\" were activated, allowing colored output for standard error (stderr).\n", false);
      }

      if(STDERR_SUPPORTS_COLORS)
      {
        LogInfo("Color meaning: ", false);
        LogError("Error |", false);
        LogWarning("| Warning", true);
      }
    }
  }

  //The following is just a remnant of an approach that no longer works, hence, this is for Windows only.
  char modulePath[1024];
  if(!GetModuleFileNameA(NULL, modulePath, ARRAY_SIZE(modulePath))) //If the first parameter is "NULL", "GetModuleFileName()" retrieves the path of the executable file of the current process.
    LogWarning("The fully qualified path for the executable file of the current process could not be obtained.", true);
  else
    LogInfo("The fully qualified path for the executable file of the current process is \"%s\".", true, modulePath);
#elif PLATFORM_POSIX
  if(isatty(STDOUT_FILENO))
  {
    STDOUT_SUPPORTS_COLORS = true; //Standard output is a "tty" (https://www.linusakesson.net/programming/tty/). In 2021, even more so in 2022, most terminal emulators support ANSI TTY escape codes.
    LogInfo("Colored output for the standard output (stdout) is active.\n", false);
    LogInfo("Color meaning: Information |", false);
    LogSuccess("| Success", true);
  }
  
  if(isatty(STDERR_FILENO))
  {
    STDERR_SUPPORTS_COLORS = true;
    LogInfo("Colored output for standard error (stderr) is active.\n", false);
    LogInfo("Color meaning: ", false);
    LogError("Error |", false);
    LogWarning("| Warning", true);
  }
#endif

  const char* configPath = "config.ini";
  if(argCount >= 2)
    configPath = argVec[1];
  else
    LogInfo("No command line arguments were provided, therefore the default configuration path \"%s\" is used.", true, configPath);

  ConfigurationLoad(configPath);

  WindowInit();
  //Ensure this is disabled on startup.
  WND->showPip = false;

  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    LogError("The initialization of GLAD failed!", true);
    glfwTerminate();

    exit(EXIT_FAILURE);
  }

  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glEnable(GL_CULL_FACE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(DEFAULT_CLEAR_COLOR[0], DEFAULT_CLEAR_COLOR[1], DEFAULT_CLEAR_COLOR[2], DEFAULT_CLEAR_COLOR[3]);

#ifndef NDEBUG
  LogInfo("<! Important Information !>\n\n"
          "To ensure debugging, even with this constrictive \"memset()\", all necessary memory is accessed, resulting in very high RAM usage.\n"
          "During execution, this will initially yield a loading time(up to a few minutes); meanwhile, the GPU remains idle due to the lack of data reception,\n"
          "which leads to a miserable frame rate. After the initial loading phase, the frame rate normalizes, though, when new chunks are created,\n"
          "the problem recurs causing short reload stutters.\nFor more information, consult the comment block of \"OwnMalloc()\" in \"Utils.h\".", true);

  //If available, set up OpenGL's debug context.
  GLint contextFlags;
  glGetIntegerv(GL_CONTEXT_FLAGS, &contextFlags);
  if(contextFlags & GLFW_OPENGL_DEBUG_CONTEXT)
  {
    LogInfo("The OpenGL debug context is currently active.", true);

    //Filter debug messages (enable anything but notifications):
    //glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE); //void glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);

    glEnable(GL_DEBUG_OUTPUT); //"KHR_debug" was published with the core version of OpenGL 4.3.
    /* Guarantee function call order. This means if we were to add a breakpoint into the definition of our callback,
     * we could traverse the call stack and locate the origin of the error. */
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLDebugCallback, NULL);
  }
#endif

  LogInfo("-=< Some Hardware and Software Information >=- \n\n", false);
  CPUInfo procInfo = GetCPUInfo(); //-> Returning an array using C: https://stackoverflow.com/a/28970797
  const char* HTT = {procInfo.hyperThreadingTech ? "Yes" : "No"};
  char logProcCores[35];
  snprintf(logProcCores, ARRAY_SIZE(logProcCores), "-> Logical processor cores: %u", procInfo.logicalProcCores);
  const char* HTTlogProcCores = {procInfo.hyperThreadingTech ? logProcCores : "-> Same as physical processor cores"};
  LogInfo("CPU: %s\nPhysical processor cores: %u\nHyper-threading: %s %s\n\n", false, procInfo.CPUBrandString, procInfo.physicalProcCores, HTT, HTTlogProcCores);

  const GLubyte* GL_Vendor = glGetString(GL_VENDOR); //The company responsible for this GL-implementation
  const GLubyte* GL_Renderer = glGetString(GL_RENDERER); //The name of the renderer, which is typically specific to a particular configuration of a hardware platform.
  LogInfo("Renderer: %s by %s\n\n", false, GL_Renderer, GL_Vendor);

  const GLubyte* GL_Version = glGetString(GL_VERSION); //Version or release number for this GL-implementation (Declaration of "GLVersion" suppresses global declaration!) 
  LogInfo("Used %sOpenGL-version%s: %s\n\n", false, BOLD, NOBOLD, GL_Version);

  const GLubyte* GLSL_Version = glGetString(GL_SHADING_LANGUAGE_VERSION); //Version or release number for the shading language
  LogInfo("Used GLSL-version: %s\n\n", false, GLSL_Version);

  LogInfo("Supported extensions:\n", false);
  GLint numExt;
  glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
  for(GLint i = 0; i < numExt; ++i)
  {
    bool notLast = i != numExt - 1;
    const char* newLine = notLast ? "\n" : ""; //-> Is "\n" considered one or two characters, and can it be stored in a char?: https://stackoverflow.com/a/23318940
    bool separator = notLast ? false : true;
    LogInfo("  - %s%s", separator, (const char*)glGetStringi(GL_EXTENSIONS, i), newLine);
  }

  //Start at the beginning of the day.
  glfwSetTime(DAY_LENGTH / 2.0);

  WindowInitFb();

  char mapPath[256];
  snprintf(mapPath, ARRAY_SIZE(mapPath), "Maps/%s", MAP_NAME);
  DatabaseInit(mapPath);
  ShaderInitAll();
  TextureInitAll();
  MapInit();
  UIInit((float)WINDOW_WIDTH / WINDOW_HEIGHT);

  Player* player = PlayerCreate();
  Camera* cam = CameraCreate(player->pos, player->pitch, player->yaw, player->front);

  //"GameObjectRefs" will be available in GLFW callback functions via "glfwGetWindowUserPointer()".
  GameObjectRefs* objects = (GameObjectRefs*)OwnMalloc(sizeof(GameObjectRefs), false);

  if(objects == NULL)
  {
    LogError("Variable \"objects\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return EXIT_FAILURE;
  }

  objects->player = player;
  glfwSetWindowUserPointer(WND->GLFW, objects);

  PlayerController* pC = PlayerControllerCreate(player);
  CameraController* cC = CameraControllerCreate(cam);
  CameraControllerSetUpdateFunc(cC, CameraControllerFirstPersonUpdate);

  ObjectLocationInfo info =
  {
    .front = player->front,
    .up = player->up,
    .pos = player->pos,
    .pitch = &player->pitch,
    .yaw = &player->yaw
  };
  CameraControllerSetTrackObject(cC, &info);

  while(!glfwWindowShouldClose(WND->GLFW))
  {
    WindowUpdateTitleFPS();

    TimeMeasurementOnNewFrame();
    float dt = (float)TimeMeasurementGet();

    Update(pC, cC, dt);

    Render(player, cam, dt);

    glfwSwapBuffers(WND->GLFW);

    WindowPollEvents();
  }

  PlayerDestroy(player);

  UIFree();
  MapFree();
  TextureFreeAll();
  ShaderFreeAll();
  DatabaseFree();

  WindowFree();

  return EXIT_SUCCESS;
}