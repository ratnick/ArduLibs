// 
// 
// 

#include "LogLib.h"
#include <TimeLib.h>

//#include <ArduinoJson.hpp>
//#include <ArduinoJson.h>
//#include <FirebaseESP8266HTTPClient.h>

String FB_FullLogPath;
int MaxLogLength;
FirebaseData* FB_firebaseData;
boolean FB_log_initialized = false;
void SendLogToFirebase();


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
		SendLogToFirebase();
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

	if (!FB_log_initialized) {
		LogLine(2,__FUNCTION__, "Begin");
		FB_FullLogPath = _FB_BasePath + "/" + _subPath + "/";
		MaxLogLength = _JSON_BUFFER_LENGTH;
		FB_firebaseData = firebaseDataPtr;
		FB_log_initialized = true;
	}
	else {
		LogLine(4, __FUNCTION__, "already initialized");
	}
}

boolean FirebaseLoggingIsInitialized() {
	return FB_log_initialized;
}

void SendLogToFirebase() {
	boolean res = true;
	int len = strlen(logStr);

	// NOTE: If Firebase makes error apparantly without reason, try to update the fingerprint in FirebaseHttpClient.h. See https://github.com/FirebaseExtended/firebase-arduino/issues/328

	if (len < MaxLogLength) {

		FirebaseJson jso;
		// clean string (https://stackoverflow.com/questions/19132867/adding-firebase-data-dots-and-forward-slashes)
		for (int i = 0; i < len; i++) {
			if (logStr[i] == '.') { logStr[i] = ','; }
			if (logStr[i] == '/') { logStr[i] = '\\'; }
			if (logStr[i] == '\n') { logStr[i] = '§'; }
		}
		logStr[0] = '|';
		logStr[len] = '|';
		logStr[len+1] = '\0';

		jso.add(logStr, "");
		res = Firebase.updateNode(*FB_firebaseData, FB_FullLogPath, jso);
		//	Serial.printf("\n%s - %d - %s", __FUNCTION__, res, logStr);
		if (res == false) {
			Serial.printf("** Log NOT written to Firebase first time: %s - length=%d - path: %s  logStr:\n|%s|\n", __FUNCTION__, strlen(logStr), FB_FullLogPath.c_str(), logStr);
		}
	}
	else {
		LogLine(1, __FUNCTION__, "**** ERROR: log too long to send to Firebase: ");
		Serial.println(logStr);
	}
}
