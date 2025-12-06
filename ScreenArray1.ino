//Code for receiver
#include <esp_now.h>
#include <WiFi.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <Wire.h>

#define HARDWARE MD_MAX72XX::GENERIC_HW
#define MAX_DEVICES 4
#define CS_PIN 5

MD_Parola displayarray1 = MD_Parola(HARDWARE, CS_PIN, MAX_DEVICES);

typedef struct message_struct{
  char text[60];
}message_struct;

message_struct incomingdata;
bool newdata = false;

void OnDataRecv(const esp_now_recv_info_t * info, const uint8_t *incomingdataptr, int len){
  memcpy(&incomingdata, incomingdataptr, sizeof(incomingdata));
  newdata = true;
}

void setup(){
  Serial.begin(115200);

  displayarray1.begin();
  displayarray1.setIntensity(5);
  displayarray1.displayClear();
  displayarray1.displayText("Waiting signal", PA_CENTER, 100, 0, PA_SCROLL_LEFT);

  WiFi.mode(WIFI_STA);
  if(esp_now_init() != ESP_OK){
    Serial.println("ESP-NOW failed");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void loop(){
  if(displayarray1.displayAnimate()){
    if(newdata){
      displayarray1.displayClear();
      displayarray1.displayText(incomingdata.text, PA_CENTER, 80, 0, PA_SCROLL_LEFT);
      newdata = false;
    }
    displayarray1.displayReset();
  }
}