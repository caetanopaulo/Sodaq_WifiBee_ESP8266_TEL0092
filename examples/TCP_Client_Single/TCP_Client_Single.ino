// TCP_Client_Single.ino

// this example use esp8266 to connect to an Access Point and connect to SINGLE TCP Server which is at the same subnet
// such as the esp8266 is is 192.168.1.3, and the server ip is 192.168.1.1 ,then esp8266 can connect to the server

#include "Sodaq_esp8266_tel0092.h"

#define  ssid    "SOMESSID"
#define password  "SOMEPASSWORD"

#define serverIP	"192.168.1.1"
#define	serverPort	"8081"

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
	SerialUSB.begin(115200);          
  Serial1.begin(115200);  
	
	init_bee();
	delay(2000);				// it will be better to delay 2s to wait esp8266 module OK
	
	 wifi.begin(&Serial1, &SerialUSB);		//Serial is used to communicate with esp8266 module, mySerial is used to debug
	if (wifi.connectAP(ssid, password)) {
		wifi.debugPrintln("connect ap sucessful !");
	} else {
		while(true);
	}
	wifi.setSingleConnect();
	if (wifi.connectTCPServer(serverIP, serverPort)) {
		wifi.debugPrintln("connect to TCP server successful !");
	}
	String ip_addr;
	ip_addr = wifi.getIP();
	wifi.debugPrintln("esp8266 ip:" + ip_addr);
	
}

void loop() {

	int state = wifi.getState();
	switch (state) {
	    case WIFI_NEW_MESSAGE: 
	      wifi.debugPrintln("new message!");
	      wifi.sendMessage(wifi.getMessage());		//send the message to TCP server what it has received
	      wifi.setState(WIFI_IDLE);
	      break;
	    case WIFI_CLOSED :							//reconnet to the TCP server 
	      wifi.debugPrintln("server is closed! and trying to reconnect it!");
	      if (wifi.connectTCPServer(serverIP, serverPort)) {
	      	wifi.debugPrintln("reconnect OK!");
	      	wifi.setState(WIFI_IDLE);
	      }
	      else {
	      	wifi.debugPrintln("reconnect fail");
	      	wifi.setState(WIFI_CLOSED);
	      }
	      break;
	    case WIFI_IDLE :							
	    	int sta = wifi.checkMessage(); 
	    	wifi.setState(sta);
	    	break;
	}

}
