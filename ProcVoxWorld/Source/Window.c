#include "Window.h"

#include <assert.h>

Window* WND;

typedef struct
{
  void* object;
  void* callbackFunc;
} UserCallbackEntry;

#define MAX_CALLBACKS 16

typedef struct
{
  UserCallbackEntry entries[MAX_CALLBACKS];
  size_t size;
} UserCallbackArray;

static UserCallbackArray FramebufferCallbacks;
static UserCallbackArray KeyboardKeyCallbacks;
static UserCallbackArray MouseButtonCallbacks;
static UserCallbackArray MouseScrollCallbacks;

void RegisterFramebufferSizeChangeCallback(void* thisObject, OnFramebufferSizeChange userCallback)
{
  if(FramebufferCallbacks.size == MAX_CALLBACKS - 1)
    assert(false), "Callbacks limit has been reached!";

  UserCallbackEntry* currEntry = &FramebufferCallbacks.entries[FramebufferCallbacks.size++];
  currEntry->object = thisObject;
  /* Standard C specifically does not support conversions between pointers to data objects and pointers to functions, but GCC and Visual Studio support it as an extension.
   * For a standard-conforming solution use a pointer to a function pointer, see: https://stackoverflow.com/a/1557103.
   * I prefer using the extension variant because it is better readable.
   * Side note: The POSIX standard, which many C compilers have to follow due to the context in which they are used,
   * stipulates that a function pointer can be converted to "void *" and back. */
#pragma warning(suppress: 4152) 
  currEntry->callbackFunc = userCallback;
}

void RegisterKeyboardKeyPressCallback(void* thisObject, OnKeyboardKeyPress userCallback)
{
  if(KeyboardKeyCallbacks.size == MAX_CALLBACKS - 1)
    assert(false), "Callbacks limit has been reached!";

  UserCallbackEntry* currEntry = &KeyboardKeyCallbacks.entries[KeyboardKeyCallbacks.size++];
  currEntry->object = thisObject;
#pragma warning(suppress: 4152) 
  currEntry->callbackFunc = userCallback;
}

void RegisterMouseButtonKeyPressCallback(void* thisObject, OnMouseButtonKeyPress userCallback)
{
  if(MouseButtonCallbacks.size == MAX_CALLBACKS - 1)
    assert(false), "Callbacks limit has been reached!";

  UserCallbackEntry* currEntry = &MouseButtonCallbacks.entries[MouseButtonCallbacks.size++];
  currEntry->object = thisObject;
#pragma warning(suppress: 4152) 
  currEntry->callbackFunc = userCallback;
}

void RegisterMouseScrollCallback(void* thisObject, OnMouseScroll userCallback)
{
  if(MouseScrollCallbacks.size == MAX_CALLBACKS - 1)
    assert(false), "Callbacks limit has been reached!";

  UserCallbackEntry* currEntry = &MouseScrollCallbacks.entries[MouseScrollCallbacks.size++];
  currEntry->object = thisObject;
#pragma warning(suppress: 4152) 
  currEntry->callbackFunc = userCallback;
}

static void SetFocused(bool isFocused)
{
  WND->isFocused = isFocused;

  const int32_t cursorMode = isFocused ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
  glfwSetInputMode(WND->GLFW, GLFW_CURSOR, cursorMode);
}

static void GLFWFramebufferSizeCallback(GLFWwindow* window, int32_t width, int32_t height)
{
  WND->width = width;
  WND->height = height;
  glViewport(0, 0, width, height);

  for(uint32_t i = 0; i < FramebufferCallbacks.size; ++i)
  {
    UserCallbackEntry* entry = &FramebufferCallbacks.entries[i];
    ((OnFramebufferSizeChange)entry->callbackFunc)(entry->object, width, height);
  }

  window; //A reference to resolve "C4100".
}

static void GLFWKeyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods)
{
  if(!WND->isFocused)
    return;

  switch(key)
  {
    case GLFW_KEY_ESCAPE:
      SetFocused(false);
      break;
    case GLFW_KEY_P:
      if(action == GLFW_PRESS)
        WND->showPip = !WND->showPip;
    break;
  };

  for(uint32_t i = 0; i < KeyboardKeyCallbacks.size; ++i)
  {
    UserCallbackEntry* entry = &KeyboardKeyCallbacks.entries[i];
    ((OnKeyboardKeyPress)entry->callbackFunc)(entry->object, key, action);
  }

  window, scancode, mods; //References to resolve "C4100".
}

static void GLFWMouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods)
{
  if(action == GLFW_PRESS)
    SetFocused(true);

  if(!WND->isFocused)
    return;

  for(uint32_t i = 0; i < MouseButtonCallbacks.size; ++i)
  {
    UserCallbackEntry* entry = &MouseButtonCallbacks.entries[i];
    ((OnMouseButtonKeyPress)entry->callbackFunc)(entry->object, button, action);
  }

  window, mods; //References to resolve "C4100".
}

static void GLFWScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
  if(!WND->isFocused)
    return;

  for(uint32_t i = 0; i < MouseScrollCallbacks.size; ++i)
  {
    UserCallbackEntry* entry = &MouseScrollCallbacks.entries[i];
    ((OnMouseScroll)entry->callbackFunc)(entry->object, (float)xOffset, (float)yOffset);
  }

  window; //A reference to resolve "C4100".
}

static void GLFWErrorCallback(int error_code, const char* description)
{
  /* Reported errors are never fatal! 
   * As long as GLFW was successfully initialized, it will remain initialized and in a safe state until terminated regardless of how many errors occur. */
  LogWarning("GLFW (%s) reports an error (%d): %s.", true, glfwGetVersionString(), error_code, description);
}

void WindowInit()
{
  if(!glfwInit())
  {
    LogError("The initialization of GLFW failed!", true);

    exit(EXIT_FAILURE);
  }
  else
    LogSuccess("GLFW (%s) has been successfully initialized.", true, glfwGetVersionString());

  //Ordain to use a particular version of OpenGL.
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR_REQUIRED);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR_REQUIRED);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

#ifdef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_FALSE);
#else
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

  GLFWmonitor* monitor = FULLSCREEN ? glfwGetPrimaryMonitor() : NULL;

  //Setup windowed full screen mode.
  if(monitor != NULL)
  {
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    glfwWindowHint(GLFW_CENTER_CURSOR, GLFW_TRUE);
  }

  WND = (Window*)OwnMalloc(sizeof(Window), false);

  if(WND == NULL)
  {
    LogError("Variable \"WND\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return;
  }

  //This will fail if the OpenGL-version is not supported.
  WND->GLFW = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, monitor, NULL);

  if(WND->GLFW == NULL)
  {
    LogError("The GLFW-window could not be initialized!", true);
    glfwTerminate();

    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(WND->GLFW);
  glfwSetInputMode(WND->GLFW, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSwapInterval(VSYNC);

  //Define callbacks:
  glfwSetFramebufferSizeCallback(WND->GLFW, GLFWFramebufferSizeCallback);
  glfwSetKeyCallback(WND->GLFW, GLFWKeyCallback);
  glfwSetMouseButtonCallback(WND->GLFW, GLFWMouseButtonCallback);
  glfwSetScrollCallback(WND->GLFW, GLFWScrollCallback);
  glfwSetErrorCallback(GLFWErrorCallback);


  WND->fb = NULL;

  WND->width = WINDOW_WIDTH;
  WND->height = WINDOW_HEIGHT;

  WND->isFocused = true;
}

void WindowInitFb()
{
  WND->fb = FramebufferCreateAll(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void WindowPollEvents()
{
  glfwPollEvents();
}

bool WindowIsKeyPressed(int32_t GLFWKeyCode)
{
  return glfwGetKey(WND->GLFW, GLFWKeyCode) == GLFW_PRESS;
}

void WindowUpdateTitleFPS()
{
  const float updateIntervalSec = 0.5f;

  static double lastTime = -1.0;
  if(lastTime < 0.0)
    lastTime = glfwGetTime();

  static int32_t numFrames = 0;
  ++numFrames;

  double currTime = glfwGetTime();
  if(currTime - lastTime >= updateIntervalSec)
  {
    int32_t FPS = (int32_t)lroundf(numFrames / updateIntervalSec);

    char title[128];
    sprintf_s(title, ARRAY_SIZE(title), "%s (%d FPS)", WINDOW_TITLE, FPS);
    glfwSetWindowTitle(WND->GLFW, title);

    numFrames = 0;
    lastTime = currTime;
  }
}

void WindowFree()
{
  FramebufferDestroyAll(WND->fb);

  glfwDestroyWindow(WND->GLFW);
  glfwTerminate();

  free(WND);
  WND = NULL;
}