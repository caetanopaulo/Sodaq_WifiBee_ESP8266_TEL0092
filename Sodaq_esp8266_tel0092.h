#ifndef _SODAQ_ESP8266_TEL0092_H_
#define _SODAQ_ESP8266_TEL0092_H_

#include <Arduino.h>
#include <Stream.h>
#include <avr/pgmspace.h>
#include <IPAddress.h>

#define	WIFI_IDLE			1
#define WIFI_NEW_MESSAGE	2
#define WIFI_CLOSED			3
#define WIFI_CLIENT_ON		4

#define WIFI_MODE_STATION			'1'
#define WIFI_MODE_AP				'2'
#define WIFI_MODE_BOTH				'3'

class Sodaq_esp8266_tel0092
{
public:
	Sodaq_esp8266_tel0092();
	void begin(Stream *serial);
	void begin(Stream *serial,Stream *serialDebug);
	void comSend();
	bool connectAP(String ssid, String password);
	//CUSTOM::BEGIN
    bool send(const uint8_t *buffer, uint32_t len);
    uint32_t recv(uint8_t *buffer, uint32_t buffer_size, uint32_t timeout = 1000); 
    bool createTCP(String addr, uint32_t port);
    bool releaseTCP(void);
	//CUSTOM::END
	bool checkEsp8266();
	bool resetEsp8266();
	void debugPrintln(String str);
	bool setSingleConnect();
	bool setMultiConnect();
	bool connectTCPServer(String serverIP, String serverPort);	
	int	 checkMessage();
	String getMessage();
	void setState(int state);
	int getState();	
	bool sendMessage(String str);
	bool sendMessage(int index, String str);
	int getWorkingID(); 
	int getFailConnectID();
	bool openTCPServer(int port, int timeout);
	bool enableAP(String ssid, String password);
	String getIP();
	bool setPureDataMode();

private:
	int available();
	void write(String str);
	void clearBuf();
	int read();
	String readData();
	void flush();	
	bool setMode(char mode);
	char checkMode();	
	bool setMux(int flag);
	//CUSTOM::BEGIN
	bool sATCIPSENDSingle(const uint8_t *buffer, uint32_t len);
	bool recvFind(String target, uint32_t timeout = 1000);
	String recvString(String target, uint32_t timeout = 1000);
	String recvString(String target1, String target2, uint32_t timeout = 1000);
	String recvString(String target1, String target2, String target3, uint32_t timeout = 1000);
	void rx_empty(void);
	uint32_t recvPkg(uint8_t *buffer, uint32_t buffer_size, uint32_t *data_len, uint32_t timeout, uint8_t *coming_mux_id);
	bool sATCIPSTARTSingle(String type, String addr, uint32_t port);
	bool eATCIPCLOSESingle(void);
	//CUSTOM::END


private:
	Stream *serial;                                            
	Stream *serialDebug;
	int connectID;
	int workingID;
	int failConnectID;
	bool multiFlag;
	int workingState;
	String message;
	char wifiMode;
	String staIP;
	String apIP;
	bool isDebug;
	bool isPureDataMode;
};

#endif 