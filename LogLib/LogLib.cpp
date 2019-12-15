// 
// 
// 

#include "LogLib.h"
#include <TimeLib.h>

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <FirebaseESP8266HTTPClient.h>

String FB_FullLogPath;
int MaxLogLength;
FirebaseData* FB_firebaseData;
boolean FB_log_initialized = false;
void SendLogToFirebase(const char* jsonStr);


int debugLevel = 0;
void InitDebugLevel(int dbgLevel) {
	debugLevel = dbgLevel;
}

int fbDebugLevel = 0;
void SetFBDebugLevel(int dbgLevel) {
	fbDebugLevel = dbgLevel;
}


String debugFunction = "";
void InitDebugFunction(const char * fncName) {
	debugFunction = String(fncName);
}

int timeAdjustmentHours = 0;

void AdjustTime(int _timeAdjustmentHours) {
	timeAdjustmentHours = _timeAdjustmentHours;
}

int GetCurrentHour() {
	time_t t = now() + (timeAdjustmentHours * 3600);
	return hour(t);
}

int GetCurrentMinute() {
	time_t t = now() + (timeAdjustmentHours * 3600);
	return minute(t);
}

int GetCurrentSecond() {
	time_t t = now() + (timeAdjustmentHours * 3600);
	return second(t);
}

uint64_t GetOnboardTime() {
	return now() + (timeAdjustmentHours * 3600);
}
String TimeString() {
	char isoTime[26];
	time_t t = now() + (timeAdjustmentHours *3600);
	sprintf(isoTime, " %02d%02d%02d %02d:%02d:%02d:%03d", year(), month(), day(), hour(t), minute(t), second(t), millis()%1000 );
	return String(isoTime);
}

void ConvertToShortTimeStr(uint64_t CurrentTime, char timeStr[]) {
	char isoTime[20];
	sprintf(timeStr, "%02d%02d%02d\0", hour(CurrentTime), minute(CurrentTime), second(CurrentTime));
	Serial.println(timeStr);
	return;
}


char logStr[1024];
char fb_logStr[1024];
void OutputLine(int dbgLevel, const char * fncName, const char* s) {   // only function actually writing to the screen

	sprintf(logStr, "%s *%d* - %s - %s\0", TimeString().c_str(), dbgLevel, fncName, s);
	if (dbgLevel <= debugLevel) {
		Serial.println(logStr);
	}
	if (FB_log_initialized && (dbgLevel <= fbDebugLevel)) {
		for (int i = 0; i < strlen(fb_logStr); i++) {   // for some strange reason, JSON format seems not to accept punctuation mark in the key. So we replace with comma.
			if (logStr[i] == '.') { logStr[i] = ','; }
		}
		sprintf(fb_logStr, "{\n\"%s\": \" \"\n}", logStr);
		SendLogToFirebase(fb_logStr);
	}
}

void LogLine(int dbgLevel, const char * fncName, const char * s) { 
	OutputLine(dbgLevel, fncName, s); 
}

void LogLinef(int dbgLevel, const char * fncName, const char * format, ...) {

	va_list arg;
	String f = String(format);
	int startStr = 0;
	int startPar = 0;
	int end = f.length();
	String s = "";
	char parFormat = ' ';


	va_start(arg, format);
	while ( (startPar = f.indexOf("%", startPar +1)) > 0) {
		s += f.substring(startStr, startPar);
		startStr = startPar + 2;
		parFormat = f[startPar + 1];
//		Serial.printf("\nStartPar=%d, startStr=%d, parFormat=%c, s=%s  VALUE=", startPar, startStr, parFormat , s.c_str());

		switch (parFormat)	{
			case 'd': s += va_arg(arg, int);
				break;
			case 'l': s += va_arg(arg, long);
				break;
			case 'f': 
				s += va_arg(arg, double);
				break;
			case 'c': s += (char)va_arg(arg, int);
				break;
			case 's': s += va_arg(arg, char*);
				break;
			default:;
		};
	}
	s += f.substring(startStr, end); // +"\n";
	va_end(arg);
	OutputLine(dbgLevel, fncName, s.c_str());
}


void InitFirebaseLogging(FirebaseData *firebaseDataPtr, String _FB_BasePath, String _subPath, int _JSON_BUFFER_LENGTH) {
	FB_FullLogPath = _FB_BasePath + "/" + _subPath + "/";
	MaxLogLength = _JSON_BUFFER_LENGTH;
	FB_firebaseData = firebaseDataPtr;
	FB_log_initialized = true;
}

void SendLogToFirebase(const char * jsonStr) {

	boolean res = true;
	// NOTE: If Firebase makes error apparantly without reason, try to update the fingerprint in FirebaseHttpClient.h. See https://github.com/FirebaseExtended/firebase-arduino/issues/328

	if (strlen(jsonStr) < MaxLogLength) {
		res = Firebase.updateNode(*FB_firebaseData, FB_FullLogPath, String(jsonStr));
		//Serial.printf("\n%s - %d - %s", __FUNCTION__, res, jsonStr);
		if (res == false) {
			Serial.printf("** Log NOT written to Firebase: %s - length=%d - Firebase error msg: %s\n%s\n", __FUNCTION__, strlen(jsonStr), FB_FullLogPath.c_str(), jsonStr);
			// will crash: 
		}
	}
	else {
		//LogLine(0, __FUNCTION__, "**** ERROR SET/PUSH FAILED: JSON too long. CRASHING ON PURPOSE");
		Serial.println(jsonStr);
		int i = 0 / 0;  // crash
	}
}
