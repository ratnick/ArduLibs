/*
  WifiLib.cpp - Library for maintaining wifi connection to a number of hotspots
  Created by Nikolaj Nøhr-Rasmussen, 2019.
  Released into the public domain.
*/

#ifndef _WIFILIB_h
#define _WIFILIB_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#include <SD.h>
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <TimeLib.h>        // http://playground.arduino.cc/code/time - installed via library manager
#include <time.h>


// NNR Library for Wifi connections

typedef struct
{
	String ssid;
	String pwd;
} hotspot_cred;

const bool INIT_HOTSPOT = true;  // false: start trying what's already stored in EEPROM from last successful connection
const int WIFI_STRENGTH_LIMIT = -80; //db

struct WifiDevice {
	int WifiIndex = 0;
	unsigned long LastWifiTime = 0;
	int WiFiConnectAttempts = 0;
	int wifiPairs = 1;
	String ssid;
	String currentSSID;
	String pwd;
};



int initWifi(struct WifiDevice *wifiDevice);
int initWifi(char _wifiSSID[], char _wifiPwd[], struct WifiDevice *wifiDevice);

static void initWifiDevice(int wifiSet);
boolean IsWifiStrenghtOK();
void PrintIPAddress();
void getCurrentTime();
boolean getCurrentTimeB();
void listNetworks();
char* GetISODateTime();
time_t getNtpTime();

#endif
