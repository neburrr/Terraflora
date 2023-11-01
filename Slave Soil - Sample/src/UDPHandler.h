#include <WebServer.h>
#include <WiFi.h>
#include <AsyncUDP.h>

AsyncUDP Udp; 

const char * masterAddress = "192.168.1.76";

unsigned int localUdpPort = 4200;  //  port to listen on
char incomingPacket[255];  // buffer for incoming packets

void parsePacket(AsyncUDPPacket packet) 
 { 
   char buf[80]; 
  
   memcpy(incomingPacket, packet.data(), sizeof(incomingPacket)); 
  
   Serial.print("Received UDP Packet Type: "); 
   Serial.println(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast"); 
   Serial.print("From: "); 
   Serial.print(packet.remoteIP()); 
   Serial.print(":"); 
   Serial.print(packet.remotePort()); 
   Serial.print(", To: "); 
   Serial.print(packet.localIP()); 
   Serial.print(":"); 
   Serial.print(packet.localPort()); 
   Serial.print(", Length: "); 
   Serial.print(packet.length()); 
   Serial.println(); 
   Serial.printf("UDP packet contents: %s\n", incomingPacket); 
  
 } 

void setupUDP(){

  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);

  if(Udp.listen(localUdpPort)){
    Udp.onPacket([](AsyncUDPPacket packet) 
     { 
       parsePacket(packet); 
     }); 
  }
     
   
}

/*
void sendUDPPacket(const char *address, const char *message){
  TelnetStream.printf("Sending: '%s', to: '%s'", message, address);
  Udp.beginPacket(address, localUdpPort);
  Udp.printf(message);
  Udp.endPacket();
}


void receiveUPDPacket(AsyncUDPPacket packet){
    int packetSize = Udp.parsePacket();
    if (packetSize){
      Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
      int len = Udp.read(incomingPacket, 255);
      if (len > 0){
        incomingPacket[len] = 0;
      }
      Serial.printf("UDP packet contents: %s\n", incomingPacket);
    } 
}*/


