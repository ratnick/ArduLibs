/*
 Name:		OTALib.h
 Created:	1/23/2021 3:25:11 PM
 Author:	Nikolaj (lokal)
 Editor:	http://www.visualmicro.com
*/

#ifndef _OTALib_h
#define _OTALib_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

void SetupOTA();
void delayNonBlocking(int ms);

#endif

