#include <WebServer.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <AsyncTCP.h>

#define SERVER_HOST_NAME "TCP_COMMS_SERVER"
#define TCP_PORT 4200

AsyncServer* TCPserver = new AsyncServer(TCP_PORT);

std::vector<AsyncClient*> clients; // a list to hold all clients
static std::vector<AsyncClient*>::iterator cl; //iterator
StaticJsonDocument<500> jsonDocument;

char incomingPacket[255];  // buffer for incoming packets

static void parseDataPre(uint8_t* data);

void jsonPostMessage(const char *tag, float value) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj[tag] = value;
}

static int getClient(const char *address){
	int cltPos = 0;
	if(!clients.empty()){
		//loop through all clients to check if they are still connected to the server
		//this procedure will update clients vector
		//if they are not connected, the item will have a 0.0.0.0 IP address
		
		for(int i = 0; i < clients.size(); i++){
			if(!strcmp(clients.at(i)->remoteIP().toString().c_str(), address)){
				clients.at(i)->add("", strlen(""));
				clients.at(i)->send();
				delay(100);
			}
		}

		for(int i = 0; i < clients.size(); i++){
			if(!strcmp(clients.at(i)->remoteIP().toString().c_str(), address))
				cltPos = i + 1;									//gets the vector last position for that address; 
																//can have the same ip in some cases, one of each will is dead item from the vector 
		}

	}
	else{
		return 0;
	}
	return cltPos;
}

void removeClient(AsyncClient* client)
{
    for(cl = clients.begin(); cl!= clients.end(); cl++)    
    {
        if( *cl == client )
        {   
            clients.erase(cl);
            return;
        }
    }
}


static void getAllClients(char* timeHour, char* timeDate, clientsStruct *clientsState){
	if(!clients.empty()){
		for(int i = 0; i < clients.size(); i++){
			clients.at(i)->add("you Alive???", strlen("you Alive???"));
			clients.at(i)->send();
			delay(100);
		}

		delay(1000);

		for(int i = 0; i < clients.size(); i++){
			TelnetStream.printf("Client: %s connected to TCP server", clients.at(i)->remoteIP().toString().c_str());
			TelnetStream.println();
			/*
			if(!strcmp(clients.at(i)->remoteIP().toString().c_str(), "0.0.0.0")){
				TelnetStream.printf("Deleting Client: %s", clients.at(i)->remoteIP().toString().c_str());
				TelnetStream.println();
				removeClient(clients.at(i));
			}*/

		}

		bool soilConnected = false;
		bool axisConnected = false;
		bool camConnected = false;

		for(int i = 0; i < clients.size(); i++){
			if(!strcmp(clients.at(i)->remoteIP().toString().c_str(), soilAddress))
				soilConnected = true;
			if(!strcmp(clients.at(i)->remoteIP().toString().c_str(), axisAddress))
				axisConnected = true;
			if(!strcmp(clients.at(i)->remoteIP().toString().c_str(), camAddress))
				camConnected = true;
		}

		if(soilConnected && !clientsState->stateSoil){
			clientsState->stateSoil = true;
			strcpy(clientsState->timeHourSoil, timeHour);
			strcpy(clientsState->timeDateSoil, timeDate);
			
		}
		else if(!soilConnected) {
			clientsState->stateSoil = false;
			strcpy(clientsState->timeHourSoil, "");
			strcpy(clientsState->timeDateSoil, "");
		}

		if(axisConnected && !clientsState->stateAxis){
			clientsState->stateAxis = true;
			strcpy(clientsState->timeHourAxis, timeHour);
			strcpy(clientsState->timeDateAxis, timeDate);
		}
		else if(!axisConnected){
			clientsState->stateAxis = false;
			strcpy(clientsState->timeHourAxis, "");
			strcpy(clientsState->timeDateAxis, "");
		}

		if(camConnected && !clientsState->stateCam){
			clientsState->stateCam = true;
			strcpy(clientsState->timeHourCam, timeHour);
			strcpy(clientsState->timeDateCam, timeDate);
		}
		else if(!camConnected){
			clientsState->stateCam = false;
			strcpy(clientsState->timeHourCam, "");
			strcpy(clientsState->timeDateCam, "");
		}
	}
	else{
		TelnetStream.println("No clients connected to TCP Server");
		clientsState->stateSoil = false;
		strcpy(clientsState->timeHourSoil, "");
		strcpy(clientsState->timeDateSoil, "");

		clientsState->stateAxis = false;
		strcpy(clientsState->timeHourSoil, "");
		strcpy(clientsState->timeDateSoil, "");

		clientsState->stateCam = false;
		strcpy(clientsState->timeHourSoil, "");
		strcpy(clientsState->timeDateSoil, "");
	}
}

static bool sendMessageToClient(const char *messageToSend, const char *address) {
	int clientNR = getClient(address);
	delay(200);
	if(clientNR == 0){
		TelnetStream.printf("%s not connected to the system", address);
		TelnetStream.println();
		return false;
	}
		
	else{
		clientNR = clientNR - 1;

		TelnetStream.printf("nrclient: %d", clientNR);
		TelnetStream.println();
	
		if (clients.at(clientNR)->space() > 255 && clients.at(clientNR)->canSend()) {
		TelnetStream.print("sending message: ");
		TelnetStream.println(messageToSend);

		clients.at(clientNR)->add(messageToSend, strlen(messageToSend));
		clients.at(clientNR)->send();
		}
		return true;
	}
}

 /* clients events */
static void handleError(void* arg, AsyncClient* client, int8_t error) {
	TelnetStream.printf(" connection error %s from client %s", client->errorToString(error), client->remoteIP().toString().c_str());
	TelnetStream.println();
}

static void handleData(void* arg, AsyncClient* client, void *data, size_t len) {
	TelnetStream.printf("Data received from client %s: ", client->remoteIP().toString().c_str());
	TelnetStream.println();
	TelnetStream.write((uint8_t*)data, len);
	TelnetStream.println();

	parseDataPre((uint8_t*)data);

	/*
	if(!strcmp((char *)data,"camUc ")){
		TelnetStream.println("cam has arrived");
		camAddress = client->remoteIP().toString().c_str();
		TelnetStream.printf("Data received from client %s, %s: ", camAddress, client->remoteIP().toString().c_str());
	}*/


  /*
	// reply to client
	if (clients.back()->space() > 32 && clients.back()->canSend()) {
		char reply[32];
		sprintf(reply, "this is from %s", SERVER_HOST_NAME);
		clients.back()->add(reply, strlen(reply));
		clients.back()->send();
	}*/
}

static void handleDisconnect(void* arg, AsyncClient* client) {
	TelnetStream.printf("client %s disconnected", client->remoteIP().toString().c_str());
	TelnetStream.println();
	removeClient(client);
}


static void handleTimeOut(void* arg, AsyncClient* client, uint32_t time) {
	//TelnetStream.printf("client ACK timeout ip: %s ", client->remoteIP().toString().c_str());
	//TelnetStream.println();
	removeClient(client);
}

static void handleNewClient(void* arg, AsyncClient* client) {
	TelnetStream.printf("new client has been connected to server, ip: %s", client->remoteIP().toString().c_str());
	TelnetStream.println();
	
	// add to list
	clients.push_back(client);

	// register events
	client->onData(&handleData, NULL);
	client->onError(&handleError, NULL);
	client->onDisconnect(&handleDisconnect, NULL);
	client->onTimeout(&handleTimeOut, NULL);
}

void setupTCP(){
	TCPserver->onClient(&handleNewClient, TCPserver);
	TCPserver->begin();
}

