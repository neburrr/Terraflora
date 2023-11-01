#include <WebServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>

#define SERVER_HOST_NAME "TCP_COMMS_SERVER"
#define TCP_PORT 4200

AsyncClient* TCPclient = new AsyncClient;

StaticJsonDocument<250> jsonDocument;

const char * masterAddress = "192.168.1.81";

static void process(String string);

void jsonPostMessage(const char *tag, float value) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj[tag] = value;
}

static void replyToServer(void* arg) {
	AsyncClient* client = reinterpret_cast<AsyncClient*>(arg);

	// send reply
	if (client->space() > 50 && client->canSend()) {
    char message[50];
		sprintf(message, "camUc");
		client->add("camUc ", strlen("camUc "));
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
	else{
		TelnetStream.println("TCP Disconnected. Restarting uC");
	Serial.println("TCP Disconnected. Restarting uC");
	ESP.restart();
	}
}

static void handleData(void* arg, AsyncClient* client, void *data, size_t len) {
	Serial.printf("\n data received from %s :\n", client->remoteIP().toString().c_str());
	Serial.write((uint8_t*)data, len);
	Serial.println();

	INPUT_STRING = String((char *)(uint8_t*)data);
	
	//process(String((char *)(uint8_t*)data));

	//parseDataPre((uint8_t*)data);

    //replyToServer(client);

	//os_timer_arm(&intervalTimer, 2000, true); // schedule for reply to server at next 2s
}

void onConnect(void* arg, AsyncClient* client) {
	Serial.printf("\n client has been connected to %s on port %d \n", SERVER_HOST_NAME, TCP_PORT);
	TCPConnected = true;
	replyToServer(client);
}

static void reconnectTCP(void* arg, AsyncClient* client){
	TelnetStream.println("TCP Disconnected. Restarting uC");
	Serial.println("TCP Disconnected. Restarting uC");
	ESP.restart();
}

void setupTCP(){
	TCPclient->connect(masterAddress, TCP_PORT);
  	TCPclient->onConnect(&onConnect, TCPclient);
  	TCPclient->onData(&handleData, TCPclient);
	TCPclient->onDisconnect(&reconnectTCP, TCPclient);
	
  	//os_timer_disarm(&intervalTimer);
    //os_timer_setfn(&intervalTimer, &replyToServer, client);
}


