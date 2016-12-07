#include "Sodaq_esp8266_tel0092.h"

Sodaq_esp8266_tel0092::Sodaq_esp8266_tel0092() {
	this->workingState = WIFI_IDLE;
	this->wifiMode = WIFI_MODE_STATION;
	this->isDebug = false;
}

void Sodaq_esp8266_tel0092::begin(Stream *serial){
	this->serial=serial;
}

void Sodaq_esp8266_tel0092::begin(Stream *serial, Stream *serialDebug){
	this->serial = serial;
	this->serialDebug = serialDebug;
	this->isDebug = true;
}

bool Sodaq_esp8266_tel0092::connectAP(String ssid, String password) {

	unsigned long timeout = 20000;
	unsigned long t_start = 0;
	int buf[10];
	char index=0;
 
	if (checkMode()!=WIFI_MODE_AP){
		this->wifiMode = WIFI_MODE_STATION;
	}
	else {
		if (setMode(WIFI_MODE_STATION))
			this->wifiMode = WIFI_MODE_STATION;
		else {
			if (this->isDebug) {
				debugPrintln("set mode to station false!");
			}
			return false;
		}
	}

	clearBuf();
	this->serial->println("AT+CWJAP=\""+ssid+"\",\""+password+"\"");
	t_start = millis();
	while ((millis()-t_start) < timeout) {
		while (available()>0) {
			buf[index] = read();
			if (buf[index]=='K' && buf[(index+9)%10]=='O') {
				return true;
			}
			if (buf[index]=='L' && buf[(index+9)%10]=='I' && buf[(index+8)%10]=='A' && buf[(index+7)%10]=='F') {
				return false;
			}
			index++;
			if (index==10)
				index = 0;
		}
	}
	if (this->isDebug) {
		debugPrintln("connect AP timeout");
	}
	return false;
}

// CUSTOM METHODS //

bool Sodaq_esp8266_tel0092::send(const uint8_t *buffer, uint32_t len)
{
    return sATCIPSENDSingle(buffer, len);
}

bool Sodaq_esp8266_tel0092::sATCIPSENDSingle(const uint8_t *buffer, uint32_t len)
{
    rx_empty();
   
    this->serial->print("AT+CIPSEND=");
    this->serial->println(len);
    if (recvFind(">", 5000)) {
        rx_empty();
        for (uint32_t i = 0; i < len; i++) {
            //write((char) buffer[i]);
             this->serial->print((char) buffer[i]);
	 		   /*wait the data send ok, clear send_buf*/
        	  flush();
        }
       
        return recvFind("SEND OK", 10000);
    }
    return false;
}

bool Sodaq_esp8266_tel0092::recvFind(String target, uint32_t timeout)
{
    String data_tmp;
    data_tmp = recvString(target, timeout);
    if (this->isDebug) {
				debugPrintln("------------ HTTP Response ------------");
				debugPrintln(data_tmp);
				debugPrintln("---------------------------------------");
			}
    if (data_tmp.indexOf(target) != -1) {
        return true;
    }
    return false;
}

String Sodaq_esp8266_tel0092::recvString(String target, uint32_t timeout)
{
    String data;
    char a;
    unsigned long start = millis();
    while (millis() - start < timeout) {
        while(available() > 0) {
            a = read();
			if(a == '\0') continue;
            data += a;
        }
        if (data.indexOf(target) != -1) {
            break;
        }   
    }
    return data;
}

String Sodaq_esp8266_tel0092::recvString(String target1, String target2, uint32_t timeout)
{
    String data;
    char a;
    unsigned long start = millis();
    while (millis() - start < timeout) {
        while(available() > 0) {
            a = read();
			if(a == '\0') continue;
            data += a;
        }
        if (data.indexOf(target1) != -1) {
            break;
        } else if (data.indexOf(target2) != -1) {
            break;
        }
    }
    return data;
}

String Sodaq_esp8266_tel0092::recvString(String target1, String target2, String target3, uint32_t timeout)
{
    String data;
    char a;
    unsigned long start = millis();
    while (millis() - start < timeout) {
        while(available() > 0) {
            a = read();
			if(a == '\0') continue;
            data += a;
        }
        if (data.indexOf(target1) != -1) {
            break;
        } else if (data.indexOf(target2) != -1) {
            break;
        } else if (data.indexOf(target3) != -1) {
            break;
        }
    }
    return data;
}

void Sodaq_esp8266_tel0092::rx_empty(void) 
{
    while(available() > 0) {
        read();
    }
}

uint32_t Sodaq_esp8266_tel0092::recv(uint8_t *buffer, uint32_t buffer_size, uint32_t timeout)
{
    return recvPkg(buffer, buffer_size, NULL, timeout, NULL);
}

/* +IPD,<id>,<len>:<data> */
/* +IPD,<len>:<data> */

uint32_t Sodaq_esp8266_tel0092::recvPkg(uint8_t *buffer, uint32_t buffer_size, uint32_t *data_len, uint32_t timeout, uint8_t *coming_mux_id)
{
    String data;
    char a;
    int32_t index_PIPDcomma = -1;
    int32_t index_colon = -1; /* : */
    int32_t index_comma = -1; /* , */
    int32_t len = -1;
    int8_t id = -1;
    bool has_data = false;
    uint32_t ret;
    unsigned long start;
    uint32_t i;
    
    if (buffer == NULL) {
        return 0;
    }
    
    start = millis();
    while (millis() - start < timeout) {
        if(available() > 0) {
            a = read();
            data += a;
        }
        
        index_PIPDcomma = data.indexOf("+IPD,");
        if (index_PIPDcomma != -1) {
            index_colon = data.indexOf(':', index_PIPDcomma + 5);
            if (index_colon != -1) {
                index_comma = data.indexOf(',', index_PIPDcomma + 5);
                /* +IPD,id,len:data */
                if (index_comma != -1 && index_comma < index_colon) { 
                    id = data.substring(index_PIPDcomma + 5, index_comma).toInt();
                    if (id < 0 || id > 4) {
                        return 0;
                    }
                    len = data.substring(index_comma + 1, index_colon).toInt();
                    if (len <= 0) {
                        return 0;
                    }
                } else { /* +IPD,len:data */
                    len = data.substring(index_PIPDcomma + 5, index_colon).toInt();
                    if (len <= 0) {
                        return 0;
                    }
                }
                has_data = true;
                break;
            }
        }
    }
    
    if (has_data) {
        i = 0;
        ret = len > buffer_size ? buffer_size : len;
        start = millis();
        while (millis() - start < 3000) {
            while(available() > 0 && i < ret) {
                a = read();
                buffer[i++] = a;
            }
            if (i == ret) {
                rx_empty();
                if (data_len) {
                    *data_len = len;    
                }
                if (index_comma != -1 && coming_mux_id) {
                    *coming_mux_id = id;
                }
                return ret;
            }
        }
    }
    return 0;
}

bool Sodaq_esp8266_tel0092::createTCP(String addr, uint32_t port)
{
    return sATCIPSTARTSingle("TCP", addr, port);
}

bool Sodaq_esp8266_tel0092::releaseTCP(void)
{
    return eATCIPCLOSESingle();
}

bool Sodaq_esp8266_tel0092::sATCIPSTARTSingle(String type, String addr, uint32_t port)
{
    String data;
    rx_empty();
    this->serial->print("AT+CIPSTART=\"");
    this->serial->print(type);
    this->serial->print("\",\"");
    this->serial->print(addr);
    this->serial->print("\",");
    this->serial->println(port);
    
    data = recvString("OK", "ERROR", "ALREADY CONNECT", 10000);
    if (data.indexOf("OK") != -1 || data.indexOf("ALREADY CONNECT") != -1) {
        return true;
    }
    return false;
}

bool Sodaq_esp8266_tel0092::eATCIPCLOSESingle(void)
{
    rx_empty();
    this->serial->println("AT+CIPCLOSE");
    return recvFind("OK", 5000);
}

/////////

int Sodaq_esp8266_tel0092::available() {
	return this->serial->available();
}

void Sodaq_esp8266_tel0092::write(String str) {
	 this->serial->println(str);
	 flush();  /*wait the data send ok, clear send_buf*/
}

bool Sodaq_esp8266_tel0092::checkEsp8266() {
	bool isOK=false;
	clearBuf();
	write("AT");
	delay(200);
	isOK = this->serial->findUntil("AT", "\r\n");
	if (true == isOK) {
		return true;
	} else {
		return false;
	}
}

/*clear rx_buf*/
void Sodaq_esp8266_tel0092::clearBuf() {
	while(available() > 0)
		read();
}

int Sodaq_esp8266_tel0092::read() {
	return this->serial->read();
}

String Sodaq_esp8266_tel0092::readData() {
	unsigned long timeout = 100;
	unsigned long t = millis();
    String data = "";
    while(millis() - t < timeout) {
    	if(available() > 0) {
	        char r = serial->read();
	        data += r;  
            t = millis();
	    }
    }
    return data;
}

bool Sodaq_esp8266_tel0092::setMode(char mode) {
	clearBuf();
	write("AT+CWMODE="+String(mode));
	delay(200);
	String str = readData();
	if (str.indexOf("no change") > 0)
		return true;
	else {
		if (resetEsp8266()) {
			this->wifiMode = mode;
			return true;
		}
		else {
			return false;
		}
	}

}

/*clear send_buf*/
void Sodaq_esp8266_tel0092::flush() {
	this->serial->flush();
}

char Sodaq_esp8266_tel0092::checkMode() {
	clearBuf();
	write("AT+CWMODE?");
	delay(200);
	String str = readData();
	// Serial.println(str);
	if (str.indexOf('1') > 0 )  
		return '1';
	else if (str.indexOf('2') > 0)
		return '2';
	else if (str.indexOf('3') > 0)
		return '3';
	else 
		return '0';
}

bool Sodaq_esp8266_tel0092::resetEsp8266() {

	unsigned long timeout = 7000;
	unsigned long t_start = 0;
	int buf[10];
	char index=0;

	clearBuf();
	write("AT+RST");
	t_start = millis();
	while ((millis()-t_start) < timeout) {
		while (available()>0) {
			buf[index] = read();
			if (buf[index]=='y' && buf[(index+9)%10]=='d' && buf[(index+8)%10]=='a' && buf[(index+7)%10]=='e' && buf[(index+6)%10]=='r') {
				return true;
			}
			index++;
			if (index==10)
				index = 0;
		}
	}
   	if (this->isDebug) {
		debugPrintln("rest Sodaq_esp8266_tel0092 timeout");
	}
	return false;	
}


void Sodaq_esp8266_tel0092::debugPrintln(String str) {
	this->serialDebug->println(str);
}


bool Sodaq_esp8266_tel0092::setMux(int flag) {
	String str;
	clearBuf();
	write("AT+CIPMUX="+String(flag));
	delay(100);
	str = readData();
	if (str.indexOf("OK")>0 || str.indexOf("link is builded")>0)
		return true;
	else 
		return false;
}

bool Sodaq_esp8266_tel0092::setSingleConnect() {
	this->connectID = 0;
	this->multiFlag = false;
	return setMux(0);
}

bool Sodaq_esp8266_tel0092::setMultiConnect() {
	this->connectID = 0;
	this->multiFlag = true;
	return setMux(1);
}

bool Sodaq_esp8266_tel0092::connectTCPServer(String serverIP, String serverPort) {
	unsigned long timeout = 5000;
	unsigned long t_start = 0;
	unsigned char buf[10];
	unsigned char index=0;

	clearBuf();
	if (!this->multiFlag) {
		write("AT+CIPSTART=\"TCP\",\"" + serverIP + "\"," + serverPort);	
		t_start = millis();
		while((millis())-t_start < timeout)	{
			while(available()) {
				buf[index] = read();
				if (buf[index]=='T' && buf[(index+9)%10]=='C' && buf[(index+8)%10]=='E' && buf[(index+7)%10]=='N'
									&& buf[(index+6)%10]=='N' && buf[(index+5)%10]=='O' && buf[(index+4)%10]=='C') {
					return true;
				}
				index++;
				if (index==10)
					index = 0;			
			}
		}
		if (this->isDebug) {	
			debugPrintln("connectTCPServer timeout");
		}
		return false;
	} else {
		write("AT+CIPSTART="+ String(this->connectID) + ",\"TCP\",\"" + serverIP + "\"," + serverPort);
		t_start = millis();
		while((millis())-t_start < timeout)	{
			while(available()) {
				buf[index] = read();
				if (buf[index]=='T' && buf[(index+9)%10]=='C' && buf[(index+8)%10]=='E' && buf[(index+7)%10]=='N'
									&& buf[(index+6)%10]=='N' && buf[(index+5)%10]=='O' && buf[(index+4)%10]=='C') {
					this->connectID++;
					return true;
				}
				index++;
				if (index==10)
					index = 0;			
			}
		}
		if (this->isDebug) {
			debugPrintln("connectTCPServer timeout");
		}
		return false;		
	}
}


int Sodaq_esp8266_tel0092::getState() {
	return this->workingState;
}

void Sodaq_esp8266_tel0092::setState(int state) {
	this->workingState = state;
}
int Sodaq_esp8266_tel0092::checkMessage() {
	String tmp="";
	tmp = readData();
	if (tmp!="") {
		if (tmp.substring(2, 6) == "+IPD") {
			if (!(this->multiFlag)) {
				int index = tmp.indexOf(":");
				int length = tmp.substring(7, index+1).toInt();
				this->message = tmp.substring(index+1, index+length+1);
				return WIFI_NEW_MESSAGE;
			} else {
				int id = 0, length=0, index=0; 
				id = tmp.substring(7, 8).toInt();
				index = tmp.indexOf(":");
				length = tmp.substring(9, index+1).toInt();
				this->workingID = id;
				this->message = tmp.substring(index+1, index+length+1);
				return WIFI_NEW_MESSAGE;
			}
		} else if (tmp.substring(0,6) == "CLOSED" || (tmp.charAt(1)==',' && tmp.substring(2,8)=="CLOSED")) {
			if (!(this->multiFlag)) {
				return WIFI_CLOSED;
			} else {
				this->failConnectID = tmp.charAt(0)-'0';
				return WIFI_CLOSED;
			}
		} else if (tmp.substring(1,9) == ",CONNECT") {
			int index = tmp.charAt(0)-'0';
			this->workingID = index;
			return WIFI_CLIENT_ON;
		} else if (this->isPureDataMode) {
			this->message = tmp;
			return WIFI_NEW_MESSAGE;
		} else {
			return WIFI_IDLE;
		}
	} else {
		return this->workingState;
	}
}

String Sodaq_esp8266_tel0092::getMessage() {
	return this->message;
}

bool Sodaq_esp8266_tel0092::sendMessage(String str) {
	if (this->isPureDataMode) {
		this->serial->print(str);
	} else {
		String tmp = "";
		int index = 0;
		int len = 0;
		len = str.length();
		write("AT+CIPSEND="+String(len));
		delay(20);
		this->serial->print(str);

		tmp = readData();
		
		if (this->isDebug) {
				debugPrintln("------------ HTTP Response ------------");
				debugPrintln(tmp);
				debugPrintln("---------------------------------------");
			}

		index = tmp.length();

		if (tmp.indexOf("SEND OK") > -1) {
			return true;
		} else {
			
			return false;
		}
	}
}

bool Sodaq_esp8266_tel0092::sendMessage(int index, String str) {
	
	String tmp = "";
	int i = 0;
	int len = 0;

	len = str.length();
	write("AT+CIPSEND="+String(index)+","+String(len));
	delay(20);
	this->serial->print(str);

	tmp = readData();
	
	if (this->isDebug) {
				debugPrintln("------------ HTTP Response ------------");
				debugPrintln(tmp);
				debugPrintln("---------------------------------------");
			}

	i = tmp.length();
	if (tmp.indexOf("SEND OK") > -1) {
		return true;
	} else {
		return false;
	}
}

int Sodaq_esp8266_tel0092::getWorkingID() {
	return this->workingID;
}

int Sodaq_esp8266_tel0092::getFailConnectID() {
	return this->failConnectID;
}

bool Sodaq_esp8266_tel0092::openTCPServer(int port, int timeout) {
	if (setMux(1)) {
		String str="";
		write("AT+CIPSERVER=1,"+String(port));
		str = readData();
		if (str.indexOf("OK")) {
			write("AT+CIPSTO="+String(timeout));
			str = readData();
			if (str.indexOf("OK")) {
				return true;
			} else {
				return false;
			}
		} else {
			return false;
		}
	} else {
		return false;
	}	
}

bool Sodaq_esp8266_tel0092::enableAP(String ssid, String password) {
	if (setMode(WIFI_MODE_AP)) {
		write("AT+CWSAP=\""+ssid+"\",\""+password+"\","+String(10)+String(4));
		String tmp;
		tmp = readData();
		if (tmp.indexOf("OK")>0) {
			return true;
		} else {
			return false;
		}
	}
}

String Sodaq_esp8266_tel0092::getIP() {
	write("AT+CIFSR");
	String tmp = readData();
	if (this->wifiMode == WIFI_MODE_STATION) {
		int index1 = tmp.indexOf("STAIP");
		int index2 = tmp.indexOf("+CIFSR:STAMAC");
		this->staIP =  tmp.substring(index1+7, index2-3);
		return this->staIP;
	} else {
		int index1 = tmp.indexOf("APIP");
		int index2 = tmp.indexOf("+CIFSR:APMAC");		
		this->apIP =  tmp.substring(index1+6, index2-3);	
		return this->apIP;
	}	
}

bool Sodaq_esp8266_tel0092::setPureDataMode() {
	write("AT+CIPMODE=1");
	String tmp = readData();
	if (tmp.indexOf("OK")>0) {
		write("AT+CIPSEND");
		this->isPureDataMode = true;
		return true;
	} else
		return false;
}