#include <SoftwareSerial.h>

SoftwareSerial mySerial(7, 8);

int numberOfRequest = 0;
const String gpsId = String("XXX");

void setup() {

	mySerial.begin(9600);
	Serial.begin(9600);

	showSerialData(20000UL);

	executeCommand("AT", 2000UL);
	executeCommand("AT", 2000UL);
	executeCommand("AT", 2000UL);
	executeCommand("AT", 2000UL);
	executeCommand("AT", 2000UL);

	executeCommand("AT+CGATT=1", 3000UL);
	executeCommand("AT+CGDCONT=1, \"IP\", \"net\"", 3000UL);
	executeCommand("AT+CGACT=1,1", 3000UL);
}

void executeCommand(const char* command, unsigned long waitTime) {
	
	mySerial.write("----Sending command----\r\n");
	mySerial.write(command);
	mySerial.write("\r\n");
	mySerial.write("-----------------------\r\n");
	
	Serial.println(command);
	showSerialData(waitTime);
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

void loop() {

	delay(5000);
	String coordinates = getCoordinates();
	sendCoordinates(coordinates);
}

String getCoordinates() {

	

	++numberOfRequest;
	return String("{ \"latitude\": \"lat") + String(numberOfRequest) + String("\", \"longitude\": \"long") + String(numberOfRequest) + String("\" }");
}

void sendCoordinates(String body) {

	String httpRequest = String("POST /api/tiolktrack/gps/XXX/location HTTP/1.1\r\n");
	httpRequest += String("Host: tiolktrack.byznass.com:31111\r\n");
	httpRequest += String("Content-Type: application/json\r\n");
	httpRequest += String("Authorization: Basic YYY\r\n");
	httpRequest += String("Content-Length: "); httpRequest += String(body.length()); httpRequest += String("\r\n\r\n");
	httpRequest += body;

	String requestCommand = String("At+cipsend="); requestCommand += String(httpRequest.length());

	executeCommand("AT+CIPSTART=\"TCP\",\"tiolktrack.byznass.com\",31111", 5000UL);
	executeCommand(requestCommand.c_str(), 2000UL);
	executeCommand(httpRequest.c_str(), 5000UL);
}
