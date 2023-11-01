#include <esp_now.h>
#include <WiFi.h>
#include <TelnetStream.h>

uint8_t masterAddress[] = {0xB8, 0xD6, 0x1A, 0xAB, 0x59, 0x2C};
uint8_t soilAddress[] = {0xF0, 0x08, 0xD1, 0xC7, 0x1F, 0xC};

typedef struct struct_message {
  uint8_t ID;
  uint8_t Type;
  uint8_t Length;
  uint16_t Checksum;
  int Info;
} struct_message;

struct_message dataPacket;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  TelnetStream.print("\r\nLast Packet Send Status:\t");
  TelnetStream.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");

  Serial.print("Bytes sent: ");
  Serial.println(sizeof(dataPacket));
  Serial.print("ID: ");
  Serial.println(dataPacket.ID);
  Serial.print("Type: ");
  Serial.println(dataPacket.Type);
  Serial.print("Lenght: ");
  Serial.println(dataPacket.Length);
  Serial.print("Checksum: ");
  Serial.println(dataPacket.Checksum);
  Serial.print("Info: ");
  Serial.println(dataPacket.Info);

  TelnetStream.print("Bytes sent: ");
  TelnetStream.println(sizeof(dataPacket));
  TelnetStream.print("ID: ");
  TelnetStream.println(dataPacket.ID);
  TelnetStream.print("Type: ");
  TelnetStream.println(dataPacket.Type);
  TelnetStream.print("Lenght: ");
  TelnetStream.println(dataPacket.Length);
  TelnetStream.print("Checksum: ");
  TelnetStream.println(dataPacket.Checksum);
  TelnetStream.print("Info: ");
  TelnetStream.println(dataPacket.Info);
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&dataPacket, incomingData, sizeof(dataPacket));

  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("ID: ");
  Serial.println(dataPacket.ID);
  Serial.print("Type: ");
  Serial.println(dataPacket.Type);
  Serial.print("Info: ");
  Serial.println(dataPacket.Info);
}

void setupESPNOW(){
    if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
  // Register peer
  memcpy(peerInfo.peer_addr, masterAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
    }
}

void sendMessage(const uint8_t *address){
  esp_err_t result = esp_now_send(address, (uint8_t *) &dataPacket, sizeof(dataPacket));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }

  else {
    Serial.println("Error sending the data");
  }
}