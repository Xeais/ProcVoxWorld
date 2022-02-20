#pragma once

#include <stdbool.h>

#define LOG_SEPARATOR "\n= = = = = = = = = = = = = = = = = = = =\n" //Exactly 20 equals signs 

extern bool STDOUT_SUPPORTS_COLORS;
extern bool STDERR_SUPPORTS_COLORS;

//Default arguments can be implemented in C, but there is no truly ideal solution: https://stackoverflow.com/questions/1472138/c-default-arguments.
void LogInfo(const char* fmtStr, bool enclosed, ...);

void LogSuccess(const char* fmtStr, bool enclosed, ...);

void LogError(const char* format, bool enclosed, ...);

void LogWarning(const char* fmtStr, bool enclosed, ...);