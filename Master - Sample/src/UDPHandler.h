#include <WebServer.h>
#include <WiFi.h>
#include <WiFiUdp.h>

WiFiUDP Udp;

const char * soilAddress = "192.168.1.81";

unsigned int localUdpPort = 4200;  //  port to listen on
char incomingPacket[255];  // buffer for incoming packets

void setupUDP(){
  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
}

void sendUDPPacket(const char *address, const char *message){
  TelnetStream.printf("Sending: '%s', to: '%s'", message, address);
  Udp.beginPacket(address, localUdpPort);
  Udp.printf(message);
  Udp.endPacket();
}

void receiveUPDPacket(){
    int packetSize = Udp.parsePacket();
    if (packetSize){
      Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
      int len = Udp.read(incomingPacket, 255);
      if (len > 0){
        incomingPacket[len] = 0;
      }
      Serial.printf("UDP packet contents: %s\n", incomingPacket);
    } 
}