// LogLib.h

#ifndef _LOGLIB_h
#define _LOGLIB_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
	
#else
	#include "WProgram.h"
#endif

#include <FirebaseESP8266.h>

void InitDebugLevel(int dbgLevel);
int GetDebugLevel();
void InitDebugFunction(const char * fncName);  // If non-zero, ´print all from this function, disregarding debug level. 
void InitFirebaseLogging(FirebaseData *firebaseDataPtr, String _FB_BasePath, String _subPath, int _JSON_BUFFER_LENGTH);
void SetFBDebugLevel(int _dbgLevel);

void LogLine(int dbgLevel, const char* fncName, const char* s);
void LogLinef(int dbgLevel, const char * fncName, const char * format, ...)  __attribute__((format(LogLinef, 2, 3)));  /* 2=format 3=params */

void AdjustTime(int _timeAdjustmentHours);
String TimeString();
uint64_t GetOnboardTime();
void ConvertToShortTimeStr(uint64_t CurrentTime, char timeStr[]);
int GetCurrentHour();
int GetCurrentMinute();
int GetCurrentSecond();

#endif
