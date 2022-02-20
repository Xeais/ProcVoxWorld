#include "Log.h"

#include "CLIFormat.h"

#include <stdio.h>
#include <stdarg.h>

void LogInfo(const char* fmtStr, bool enclosed, ...)
{
  if(STDOUT_SUPPORTS_COLORS)
    fprintf(stdout, "%s", FG_CYAN);
  else
    fprintf(stdout, "%s", "INFORMATION | ");

  //It is unfortunately not possible to outsource the rest to a common function.
  va_list argPtr;
  va_start(argPtr, enclosed); //-> "How to remove this warning: second parameter of ‘va_start’ not last named argument?": https://stackoverflow.com/a/13189394 | Microsoft should really finally update their C-compiler!
  vfprintf(stdout, fmtStr, argPtr);
  va_end(argPtr);

  if(STDOUT_SUPPORTS_COLORS)
    fprintf(stdout, "%s", FG_RESET);

  if(enclosed)
    fprintf(stdout, "%s", LOG_SEPARATOR);
}

void LogSuccess(const char* fmtStr, bool enclosed, ...)
{
  if(STDOUT_SUPPORTS_COLORS)
    fprintf(stdout, "%s", FG_GREEN);
  else
    fprintf(stdout, "%s", "SUCCESS | ");

  va_list argPtr;
  va_start(argPtr, enclosed);
  vfprintf(stdout, fmtStr, argPtr);
  va_end(argPtr);

  if(STDOUT_SUPPORTS_COLORS)
    fprintf(stdout, "%s", FG_RESET);

  if(enclosed)
    fprintf(stdout, "%s", LOG_SEPARATOR);
}

void LogError(const char* fmtStr, bool enclosed, ...)
{
  if(STDERR_SUPPORTS_COLORS)
    fprintf(stderr, "%s", FG_RED);
  else
    fprintf(stderr, "%s", "ERROR | ");

  va_list argPtr;
  va_start(argPtr, enclosed); 
  vfprintf(stderr, fmtStr, argPtr);
  va_end(argPtr);

  if(STDERR_SUPPORTS_COLORS)
    fprintf(stderr, "%s", FG_RESET);

  if(enclosed)
    fprintf(stderr, "%s", LOG_SEPARATOR);
}

void LogWarning(const char* fmtStr, bool enclosed, ...)
{
  if(STDERR_SUPPORTS_COLORS)
    fprintf(stderr, "%s", FG_YELLOW); //-> "Should I output warnings to STDERR or STDOUT?": https://stackoverflow.com/a/1430971
  else
    fprintf(stderr, "%s", "WARNING | ");

  va_list argPtr;
  va_start(argPtr, enclosed);
  vfprintf(stderr, fmtStr, argPtr);
  va_end(argPtr);

  if(STDERR_SUPPORTS_COLORS)
    fprintf(stderr, "%s", FG_RESET);

  if(enclosed)
    fprintf(stderr, "%s", LOG_SEPARATOR);
}