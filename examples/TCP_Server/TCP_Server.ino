// TCP_Server.ino
// this example use esp8266 to connect to an Access Point and connect to multiple TCP Server which is at the same subnet.
// such as the esp8266 is is 192.168.1.3, and the server ip is 192.168.1.1 ,then esp8266 can connect to the server

#include "Sodaq_esp8266_tel0092.h"

#define  ssid    "SOMESSID"
#define password  "SOMEPASSWORD"

#define	serverPort	8081		// esp8266 will open it's 8081 port , and other can connect 

Sodaq_esp8266_tel0092 wifi;

void init_bee() {
   pinMode(BEE_VCC, OUTPUT);
   digitalWrite(BEE_VCC, HIGH);
   delay(2);
   pinMode(BEEDTR, OUTPUT);
   digitalWrite(BEEDTR, HIGH);
   
   pinMode(BEECTS, INPUT);
  }

void setup() {
  init_bee();
  
	delay(2000);				// it will be better to delay 2s to wait esp8266 module OK
  
	SerialUSB.begin(115200);          
  Serial1.begin(115200);
  
	 wifi.begin(&Serial1, &SerialUSB);		//Serial is used to communicate with esp8266 module, mySerial is used to debug
	if (wifi.connectAP(ssid, password)) {
		wifi.debugPrintln("connect ap sucessful !");
	} else {
		while(true);
	}
	if (wifi.setMultiConnect()) {
		wifi.debugPrintln("set multi connect!");
	}	
	if (wifi.openTCPServer(serverPort, 180)) {  //180 secondes, if a client send no message in 180s, server will close it
		wifi.debugPrintln("open TCP Server port "+ String(serverPort) + " OK!");
	}	
	wifi.debugPrintln("Server IP:" + wifi.getIP() + " Port:" + String(serverPort));

}

void loop() {

	int state = wifi.getState();
	switch (state) {
	    case WIFI_NEW_MESSAGE: 
	      wifi.debugPrintln(String(wifi.getWorkingID()) + ":" + wifi.getMessage()); // debug
	      wifi.sendMessage(wifi.getWorkingID(), wifi.getMessage());	// send the message back
	      wifi.setState(WIFI_IDLE);
	      break;
	    case WIFI_CLOSED : 	// just print which connect is close, won't reconnect
	      wifi.debugPrintln(String(wifi.getFailConnectID()) + ":connect closed!");
	      wifi.setState(WIFI_IDLE);
	      break;
	    case WIFI_IDLE :
	    {
	    	int state = wifi.checkMessage(); 
	    	wifi.setState(state);
	    	break;
	    }
	    case WIFI_CLIENT_ON :	//if a client is connected ,say hello to it
	    	wifi.sendMessage(wifi.getWorkingID(), "hello, this is esp8266.");
	    	wifi.setState(WIFI_IDLE);
	    	break;
	}

}
