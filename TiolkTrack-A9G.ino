#include <SoftwareSerial.h>
#include <string.h>

SoftwareSerial mySerial(7, 8);

int numberOfRequest = 0;
const String gpsId = String("XXX");

void setup() {

	mySerial.begin(9600);
	Serial.begin(9600);

	showSerialData(20000UL);

	executeCommandWithEcho("AT", 2000UL);
	executeCommandWithEcho("AT", 2000UL);
	executeCommandWithEcho("AT", 2000UL);
	executeCommandWithEcho("AT", 2000UL);
	executeCommandWithEcho("AT", 2000UL);

	executeCommandWithEcho("AT+CGATT=1", 3000UL);
	executeCommandWithEcho("AT+CGDCONT=1, \"IP\", \"net\"", 3000UL);
	executeCommandWithEcho("AT+CGACT=1,1", 3000UL);

	executeCommandWithEcho("AT+GPS=1", 5000UL);
}

void loop() {

	delay(5000);
	char* coordinates = getCoordinates();

	if(coordinates != NULL) {
		sendCoordinates(coordinates);
		free(coordinates);
	}
	
}

char* getCoordinates() {

	char* result = executeCommandWithResult("AT+LOCATION = 2", 2000UL);

	// Search for 'F' letter that is present in error message
	if(result[0] == 0 || strchr(result, 'F') != NULL){
		free(result);
		return NULL;
	}
	
	char* json = (char*)malloc(64 * sizeof(char));
	
	strcpy(json, "{ \"latitude\": \"");
	
	char * pch;
	pch = strtok(result, "\n");
	pch = strtok(NULL, "\n");
	pch = strtok(NULL, "\n,");

	strcat(json, pch);
	
	strcat(json, "\", \"longitude\": \"");
	
	pch = strtok(NULL, "\r,");
	strcat(json, pch);
	
	strcat(json, "\" }");
	
	free(result);

	return json;
}

void sendCoordinates(String body) {

	String httpRequest = String("POST /api/tiolktrack/gps/XXX/location HTTP/1.1\r\n");
	httpRequest += String("Host: tiolktrack.byznass.com:31111\r\n");
	httpRequest += String("Content-Type: application/json\r\n");
	httpRequest += String("Authorization: Basic YYY\r\n");
	httpRequest += String("Content-Length: "); httpRequest += String(body.length()); httpRequest += String("\r\n\r\n");
	httpRequest += body;

	String requestCommand = String("At+cipsend="); requestCommand += String(httpRequest.length());

	executeCommandWithEcho("AT+CIPSTART=\"TCP\",\"tiolktrack.byznass.com\",31111", 5000UL);
	executeCommandWithEcho(requestCommand.c_str(), 2000UL);
	executeCommandWithEcho(httpRequest.c_str(), 5000UL);
}

void executeCommandWithEcho(const char* command, unsigned long waitTime) {
	
	executeCommand(command);
	showSerialData(waitTime);
}

char* executeCommandWithResult(const char* command, unsigned long waitTime) {
	
	executeCommand(command);

	char* result = getSerialData(waitTime);
	mySerial.print(result);

	return result;
}

void executeCommand(const char* command) {
	
	mySerial.write("----Sending command----\r\n");
	mySerial.write(command);
	mySerial.write("\r\n");
	mySerial.write("-----------------------\r\n");
	
	Serial.println(command);
}

void showSerialData(unsigned long waitTime){

	unsigned long time = millis();
	unsigned long delta = 0;

	while (delta < waitTime){
		while (Serial.available() != 0) {
			mySerial.write(Serial.read());
		}

		delta = millis() - time;
	}
}

char* getSerialData(unsigned long waitTime) {

	unsigned long time = millis();
	unsigned long delta = 0;
	
	char* buff = (char*)malloc(64 * sizeof(char));
	int pos = 0;
	while (delta < waitTime){
		while (Serial.available() != 0) {
			buff[pos++] = (char) Serial.read();
		}

		delta = millis() - time;
	}

	buff[pos] = 0;

	return buff;
}
