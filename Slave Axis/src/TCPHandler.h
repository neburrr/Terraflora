#include <WebServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>

#define SERVER_HOST_NAME "TCP_COMMS_SERVER"
#define TCP_PORT 4200

AsyncClient* TCPclient = new AsyncClient;

StaticJsonDocument<250> jsonDocument;

const char * masterAddress = "192.168.1.81";

static void parseDataPre(uint8_t* data);
static void process(String string);
void move_to(float x, float y);
void homeSequence();

void jsonPostMessage(const char *tag, float value) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj[tag] = value;
}

static void replyToServer(void* arg) {
	AsyncClient* client = reinterpret_cast<AsyncClient*>(arg);

	// send reply
	if (client->space() > 50 && client->canSend()) {
    char message[50];
		sprintf(message, "this is from %s , 'axisUc'", WiFi.localIP().toString().c_str());
		client->add(message, strlen(message));
		client->send();
	}
}

static void sendMessageToServer(char *messageToSend) {
	Serial.print("sending message: ");
	Serial.println(messageToSend);
	
	if (TCPclient->space() > 255 && TCPclient->canSend()) {
		TelnetStream.print("sending message: ");
	  	TelnetStream.println(messageToSend);

		TCPclient->add(messageToSend, strlen(messageToSend));
		TCPclient->send();
	}
}

String toString(uint8_t *str){
    return String((char *)str);
}



static void handleData(void* arg, AsyncClient* client, void *data, size_t len) {
	Serial.printf("\n data received from %s :\n", client->remoteIP().toString().c_str());
	Serial.write((uint8_t*)data, len);
	Serial.println();

	/*
	Serial.println("homing2");

	if(!strcmp((char*)data, "you Alive???")){
		Serial.println("homing");
	}
	Serial.println("homing3");

	if(strcmp((char*)data, "you Alive???")){
	
	Serial.println("homing4");
	DynamicJsonDocument doc(256);
	deserializeJson(doc, data);

	const char * mode = doc["mode"];
	Serial.println("homing5");
	if (!strcmp(mode, "G00")){
		int x = doc["X"];
		int y = doc["Y"];
		Serial.printf("moving to x: %d - y: %d\n", x, y);

		move_to(x,y);
	}
	else if(!strcmp(mode, "G28")){

		Serial.println("homing");

		homeSequence();
	}
	}
	Serial.println("homing6");*/
	INPUT_STRING = String((char *)(uint8_t*)data);


	//process(String((char *)(uint8_t*)data));

	//parseDataPre((uint8_t*)data);

    //replyToServer(client);

	//os_timer_arm(&intervalTimer, 2000, true); // schedule for reply to server at next 2s
}

void onConnect(void* arg, AsyncClient* client) {
	Serial.printf("\n client has been connected to %s on port %d \n", SERVER_HOST_NAME, TCP_PORT);
	replyToServer(client);
}

static void reconnectTCP(void* arg, AsyncClient* client){
	Serial.println("TCP Disconnected. Restarting uC");
	ESP.restart();
}
static void error(void* arg, AsyncClient* client){
	Serial.println("TCP error");

}

void setupTCP(){
	TCPclient->connect(masterAddress, TCP_PORT);
  	TCPclient->onConnect(&onConnect, TCPclient);
  	TCPclient->onData(&handleData, TCPclient);
	TCPclient->onDisconnect(&reconnectTCP, TCPclient);

  	//os_timer_disarm(&intervalTimer);
    //os_timer_setfn(&intervalTimer, &replyToServer, client);
}


