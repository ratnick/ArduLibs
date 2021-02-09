#include "WifiLib.h"
#include "OTALib.h"

//#include <time.h>



// Wifi and network globals
const char timeServer[] = "0.dk.pool.ntp.org"; // Danish NTP Server 
WiFiClientSecure wifiClient;
byte macAddr[6];
//WiFiClient wifiClient;
const int MAX_CONNECT_RETRIES = 2; 

#include "wifipasswords.h"

/* The file wifipasswords.h is private and has the following format:
const int MAX_HOTSPOTS_DEFINED = 7;  // nbr of hotspots below
hotspot_cred SSID_pairs[MAX_HOTSPOTS_DEFINED] = {
	{ "nohrx-yy", "*******" },
	{ "nohrx", ""*******" },
	{ "nohrx_2Gxxx", ""*******" },
	{ "nohrx_xxx", ""*******" },
	{ "CAR HOTSPOT", ""*******" },
	{ "ROOF HOTSPOT", ""*******" },
	{ "Nikxxx", ""*******" }
};
*/

// NTPTimeServices
WiFiUDP NtpUdp;
unsigned int localNtpPort = 8888;  // local port to listen for UDP packets
const int timeZone = 0;
char isoTime[30];

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte ntpPacketBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets




static void initWifiDevice(int wifiSet, struct WifiDevice *wifiDevice) {   //wifiset always zero at the moment. But in time, it will be used for retries at different wifi ssid
	Serial.print("wifiSet="); //NNR
	Serial.println(wifiSet);
	wifiDevice->currentSSID = SSID_pairs[wifiSet].ssid;
	wifiDevice->pwd = SSID_pairs[wifiSet].pwd;
}

int initWifi(struct WifiDevice *wifiDevice) {
	// return values:
	//  200: success. Is already connected.
	//	100: success connecting to SSID + pwd given in parameters
	//  0-N: success in connecting to (SSID, pwd) tuple number n.
	//   -1: no success in connecting.
	return initWifi("", "", wifiDevice);
}

int initWifi(char _wifiSSID[], char _wifiPwd[], struct WifiDevice *wifiDevice) {
	const int WifiTimeoutMilliseconds = 60000;  // 60 seconds
	const int MaxRetriesWithSamePwd = 20;
	int MaxLoopsThroughAll = MAX_CONNECT_RETRIES;
	int retry;
	char wifiSSID[50]; char wifiPwd[50];

	WiFi.mode(WIFI_STA);  // Ensure WiFi in Station/Client Mode

	if (WiFi.status() == WL_NO_SHIELD) {
		Serial.println("WiFi shield not present. TERMINATING");
		while (true);  // don't continue
	}

	if ((WiFi.status() == WL_CONNECTED) && IsWifiStrenghtOK()) {
		return 200;
	}
	else {

		// listNetworks();
		while (MaxLoopsThroughAll-- >= 0) {
			wifiDevice->WifiIndex = 0;
			Serial.println("Not connected. Trying all known wifi hotspots.");

			if (_wifiSSID[0] != 0) {
				Serial.print("initWifi: disconnect before retrying - ");
				WiFi.disconnect();
				Serial.print("initWifi: trying the SSID and pwd stored in EEPROM: "); // +String(_wifiSSID));
				Serial.println(_wifiSSID);
				WiFi.begin(_wifiSSID, _wifiPwd);

				// NNR: We should not proceed before we are connected to a wifi
				delayNonBlocking(500);
				retry = 0;
				while (WiFi.status() != WL_CONNECTED && retry++ < MaxRetriesWithSamePwd) {
					delayNonBlocking(500);
					Serial.print(".");
				}

				if (WiFi.status() == WL_CONNECTED && IsWifiStrenghtOK()) {
					Serial.print(" => connected OK with already stored ssid/pwd: ");
					Serial.println(_wifiSSID);
					return 100;
				}
			}

			while (wifiDevice->WifiIndex <= wifiDevice->wifiPairs) {

				if (wifiDevice->LastWifiTime > millis()) { delayNonBlocking(500); }

				initWifiDevice(wifiDevice->WifiIndex, wifiDevice);
				Serial.print("initWifi: trying: " + wifiDevice->currentSSID + " WifiIndex="); 
				Serial.println(wifiDevice->WifiIndex);
				WiFi.begin(wifiDevice->currentSSID.c_str(), wifiDevice->pwd.c_str());
				Serial.print(" .. begin:");

				// NNR: We should not proceed before we are connected to a wifi
				delayNonBlocking(500);
				retry = 0;
				while (WiFi.status() != WL_CONNECTED && retry++ < MaxRetriesWithSamePwd) {
					delayNonBlocking(500);
					Serial.print(".");
				}

				if (WiFi.status() == WL_CONNECTED && IsWifiStrenghtOK()) {
					Serial.print(" => connected OK to: ");
					Serial.println(_wifiSSID);
					return wifiDevice->WifiIndex;
				}
				else {
					Serial.println(" could not connect");
					wifiDevice->WiFiConnectAttempts++;
					wifiDevice->LastWifiTime = millis() + WifiTimeoutMilliseconds;
					//if (wifiDevice.WifiIndex++ > wifiDevice.wifiPairs) { wifiDevice.WifiIndex = 1; }
				}
				wifiDevice->WifiIndex++;
			}
		}
	}

	Serial.println("*** Could not connect to Wifi at all. Try 1) power cycling. 2) look if your SSID is defined in the list.");
	delayNonBlocking(10000);
	return -1;
}

boolean IsWifiStrenghtOK() {
	int strength = WiFi.RSSI();
	//Serial.printf("\n Wifi strength = %i\n", strength);
//	LogLinef(4, __FUNCTION__, "Wifi strength = %ddB", strength);
	return (WiFi.RSSI() > WIFI_STRENGTH_LIMIT);
}

String GetIpAddress()
{
	IPAddress ipAddress = WiFi.localIP();
	return String(ipAddress[0]) + String(".") + \
		String(ipAddress[1]) + String(".") + \
		String(ipAddress[2]) + String(".") + \
		String(ipAddress[3]);
}

void PrintIPAddress() {
/*
	int ipAddress;
	byte ipQuads[4];

	ipAddress = WiFi.localIP();
	ipQuads[0] = (byte)(ipAddress & 0xFF);;
	ipQuads[1] = (byte)((ipAddress >> 8) & 0xFF);
	ipQuads[2] = (byte)((ipAddress >> 16) & 0xFF);
	ipQuads[3] = (byte)((ipAddress >> 24) & 0xFF);
	Serial.println("Connected with ip address: " + String(ipQuads[0]) + "." + String(ipQuads[1]) + "." + String(ipQuads[2]) + "." + String(ipQuads[3]));
*/
	//print the local IP address
	Serial.print("Connected with ip address: ");
	Serial.print(WiFi.localIP());
	Serial.print(" on SSID: ");
	Serial.println(WiFi.SSID());
}

void getCurrentTime() {
	boolean res = getCurrentTimeB();
	//LogLine(4, __FUNCTION__, "NTP time fetched: " + String(res));
}

boolean getCurrentTimeB() {
	int ntpRetryCount = 0;
	int max_retries = 3;

	while (timeStatus() == timeNotSet && ntpRetryCount < max_retries) { // get NTP time
		Serial.println(WiFi.localIP());
		setSyncProvider(getNtpTime);
		setSyncInterval(60 * 60);
		ntpRetryCount++;
	}
//	LogLinef(4, __FUNCTION__, "NTP time fetched in %d tries", ntpRetryCount);
	return (ntpRetryCount < max_retries);
}

void listNetworks() {
	// scan for nearby networks:
	Serial.println("** Scan Networks **");
	int numSsid = WiFi.scanNetworks();
	if (numSsid == -1) {
		Serial.println("Couldn't get a wifi connection");
		while (true);
	}

	// print the list of networks seen:
	Serial.print("number of available networks:");
	Serial.println(numSsid);

	// print the network number and name for each network found:
	for (int thisNet = 0; thisNet < numSsid; thisNet++) {
		Serial.print(thisNet);
		Serial.print(") ");
		Serial.print(WiFi.SSID(thisNet));
		Serial.print("\tSignal: ");
		Serial.print(WiFi.RSSI(thisNet));
		Serial.println(" dBm");
		//	Serial.print("\tEncryption: ");
		//	printEncryptionType(WiFi.encryptionType(thisNet));
	}
}


// NTPTimeServices

// send an NTP request to the time server at the given address
//void sendNTPpacket(IPAddress &address)
void sendNTPpacket(const char* host)
{
	// set all bytes in the buffer to 0
	memset(ntpPacketBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	ntpPacketBuffer[0] = 0b11100011;   // LI, Version, Mode
	ntpPacketBuffer[1] = 0;     // Stratum, or type of clock
	ntpPacketBuffer[2] = 6;     // Polling Interval
	ntpPacketBuffer[3] = 0xEC;  // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	ntpPacketBuffer[12] = 49;
	ntpPacketBuffer[13] = 0x4E;
	ntpPacketBuffer[14] = 49;
	ntpPacketBuffer[15] = 52;
	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:                 
	NtpUdp.beginPacket(host, 123); //NTP requests are to port 123
	NtpUdp.write(ntpPacketBuffer, NTP_PACKET_SIZE);
	NtpUdp.endPacket();
}

char* GetISODateTime() {
	sprintf(isoTime, "%4d-%02d-%02dT%02d:%02d:%02d", year(), month(), day(), hour(), minute(), second());
	return isoTime;
}

time_t getNtpTime()
{
	NtpUdp.begin(localNtpPort);

	while (NtpUdp.parsePacket() > 0); // discard any previously received packets
	Serial.print("Transmit NTP Request...");
	sendNTPpacket(timeServer);
	uint32_t beginWait = millis();
	while (millis() - beginWait < 1500) {
		int size = NtpUdp.parsePacket();
		if (size >= NTP_PACKET_SIZE) {
			Serial.println(".. got NTP Response");
			NtpUdp.read(ntpPacketBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
			unsigned long secsSince1900;
			// convert four bytes starting at location 40 to a long integer
			secsSince1900 = (unsigned long)ntpPacketBuffer[40] << 24;
			secsSince1900 |= (unsigned long)ntpPacketBuffer[41] << 16;
			secsSince1900 |= (unsigned long)ntpPacketBuffer[42] << 8;
			secsSince1900 |= (unsigned long)ntpPacketBuffer[43];

			NtpUdp.stop();

			return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
		}
	}
	NtpUdp.stop();

	Serial.println("..No NTP Response.");
	return 0; // return 0 if unable to get the time
}

