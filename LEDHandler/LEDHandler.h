/*
  LEDHandler.cpp - Library for simple LED handling
  Created by Nikolaj Nøhr-Rasmussen, 2019.
  Released into the public domain.
*/

#ifndef _LEDHANDLER_h
#define _LEDHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#endif

void initFlashLED();
void LED_ON();
void LED_OFF();
void LED_Flashes(int count, int blinkDelayMs);