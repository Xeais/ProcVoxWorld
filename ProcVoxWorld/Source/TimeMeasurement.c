#include "TimeMeasurement.h"

#define GLFW_INCLUDE_NONE //Prevent the GLFW-header from including the OpenGL-header.
#include "GLFW/glfw3.h"

#include <stdbool.h>
#include <assert.h>

static bool noLastTime = true;
static double lastTime = 0.0;
static double dt = 0.0;

void TimeMeasurementOnNewFrame()
{
  if(noLastTime)
  {
    lastTime = glfwGetTime();
    noLastTime = 0;
  }

  double currTime = glfwGetTime();
  dt = currTime - lastTime;
  lastTime = currTime;
}

double TimeMeasurementGet()
{
  assert(!noLastTime);

  return dt;
}