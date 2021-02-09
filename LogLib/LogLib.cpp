// 
// 
// 

#include "LogLib.h"
#include "OTALib.h"
#include <TimeLib.h>
#include "FirebaseLib.h"

String FB_FullLogPath;
int MaxLogLength;
FirebaseData FB_firebaseDataCopy;

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
void OutputLine(int dbgLevel, const char * fncName, const char* s, bool sendToCloud) {   // only function actually writing to the screen
	sprintf(logStr, "%s *%d* - %s - %s\0", TimeString().c_str(), dbgLevel, fncName, s);
	if (dbgLevel <= debugLevel) {
		Serial.println(logStr);
	}
	if (sendToCloud && FB_log_initialized && (dbgLevel <= fbDebugLevel)) {
		SendLogToFirebase();
	}
}

void OutputLine(int dbgLevel, const char* fncName, const char* s) {
	OutputLine(dbgLevel, fncName, s, true);
}

void LogLine(int dbgLevel, const char * fncName, const char * s) {
	OutputLine(dbgLevel, fncName, s, true); 
}

void LogLine(int dbgLevel, const char* fncName, const char* s, bool sendToCloud) {
	OutputLine(dbgLevel, fncName, s, sendToCloud);
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

//nnr		Serial.printf("\nStartPar=%d, startStr=%d, parFormat=%c, s=%s  VALUE=", startPar, startStr, parFormat , s.c_str());

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

void InitFirebaseLogging(FirebaseData &firebaseData, String _FB_BasePath, String _subPath, int _JSON_BUFFER_LENGTH) {

	LogLinef(4, __FUNCTION__, "Begin: fbDebugLevel=%d, FB_log_initialized=%d", fbDebugLevel, FB_log_initialized);
	if (fbDebugLevel >= 0) {
		if (!FB_log_initialized)  {
			LogLine(2,__FUNCTION__, "Begin");
			FB_FullLogPath = _FB_BasePath + "/" + _subPath + "/";
			MaxLogLength = _JSON_BUFFER_LENGTH;
			FB_firebaseDataCopy = firebaseData;
			FB_log_initialized = true;
		}
		else {
			LogLine(4, __FUNCTION__, "already initialized");
		}
	}
	else {
		FB_log_initialized = false;
	}
	LogLinef(4, __FUNCTION__, "  End: fbDebugLevel=%d, FB_log_initialized=%d", fbDebugLevel, FB_log_initialized);
}

boolean FirebaseLoggingIsInitialized() {
	return FB_log_initialized;
}

void SendLogToFirebase() {
	bool res = true;
	bool res2 = true;
	int len = strlen(logStr);

	// NOTE: If Firebase makes error apparantly without reason, try to update the "secret" from Firebase in FIREBASE_AUTH 
	if (FirebaseLoggingIsInitialized()) {
		if (len < MaxLogLength - 1) {

			FirebaseJson jso;
			// clean string (https://stackoverflow.com/questions/19132867/adding-firebase-data-dots-and-forward-slashes)
			for (int i = 0; i < len; i++) {
				if (logStr[i] == '.') { logStr[i] = ','; }
				if (logStr[i] == '/') { logStr[i] = '|'; }
				if (logStr[i] == '\n') { logStr[i] = '§'; }
				if (logStr[i] == '[') { logStr[i] = '{'; }
				if (logStr[i] == ']') { logStr[i] = '}'; }
				if (logStr[i] == '$') { logStr[i] = '§'; }
			}
			logStr[0] = '|';
			logStr[len] = '|';
			logStr[len + 1] = '\0';

			jso.add(logStr, "");  // Note the key is the actual string. The data in that key is empty.
			String jsoStr;
			(jso).toString(jsoStr, true);

			//nnr this should not be necessary: delayNonBlocking(300);  // we need to make it doesn't clog. Simple solution TODO a better one.
			res = Firebase.updateNode(FB_firebaseDataCopy, FB_FullLogPath, jsoStr);
			//	Serial.printf("\n%s - %d - %s", __FUNCTION__, res, logStr);
			if (!res) {
				Serial.printf("** Log NOT written to Firebase 1st time: %s - length=%d - path: %s  logStr:\n>%s<\n\n", __FUNCTION__, strlen(logStr), FB_FullLogPath.c_str(), logStr);
				Serial.println(FB_FullLogPath);
				Serial.println(jsoStr);
				//delayNonBlocking(500);
				//jso.add(logStr, "");
				//res2 = Firebase.updateNode(*FB_firebaseDataptr, FB_FullLogPath, jso);
			}
		}
		else {
			LogLine(1, __FUNCTION__, "**** ERROR: log too long to send to Firebase: ");
			Serial.println(logStr);
		}
	}
	else {
		LogLine(0, __FUNCTION__, "**** ERROR: Firebase logging not initialized yet.");
		Serial.println(logStr);
	}

}
