// TCP_Client_multi.ino

// this example use esp8266 to connect to an Access Point and connect to multiple TCP Server which is at the same subnet.
// such as the esp8266 is is 192.168.1.2, and the server ip is 192.168.1.1 ,then esp8266 can connect to the server

#include "Sodaq_esp8266_tel0092.h"

#define  ssid    "SOMESSID"
#define password  "SOMEPASSWORD"

#define serverIP1	"192.168.1.1"
#define	serverPort1	"8081"

#define serverIP2	"192.168.1.1"
#define	serverPort2	"8082"

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
  
	SerialUSB.begin(115200);          
  Serial1.begin(115200);  

  delay(2000);        // it will be better to delay 2s to wait esp8266 module OK

  
	wifi.begin(&Serial1, &SerialUSB);  //Serial is used to communicate with esp8266 module, mySerial is used to print debug message

	if (wifi.connectAP(ssid, password)) {
		wifi.debugPrintln("connect ap successful !");
	} else {
		while(true);
	}

	wifi.setMultiConnect();
	if (wifi.connectTCPServer(serverIP1, serverPort1)) {
		wifi.debugPrintln("connect to TCP Server 1");
	}
	if (wifi.connectTCPServer(serverIP2, serverPort2)) {
		wifi.debugPrintln("connect to TCP Server 2");
	}

}

void loop() {

	int state = wifi.getState();
	switch (state) {
	    case WIFI_NEW_MESSAGE: 
	      wifi.debugPrintln(String(wifi.getWorkingID()) + ":" + wifi.getMessage()); //debug 
	      wifi.sendMessage(wifi.getWorkingID(), wifi.getMessage());	//sent the message back;
	      wifi.setState(WIFI_IDLE);
	      break;
	    case WIFI_CLOSED : 		// just print which connect is close, won't reconnect
	      wifi.debugPrintln(String(wifi.getFailConnectID()) + ":connect closed!"); 
	      wifi.setState(WIFI_IDLE);
	      break;
	    case WIFI_IDLE :
	    	int state = wifi.checkMessage(); 
	    	wifi.setState(state);
	    	break;
	}

}
