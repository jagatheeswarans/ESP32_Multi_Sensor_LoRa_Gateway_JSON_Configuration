#include "FS.h"
#include "SPIFFS.h"

#include <SPI.h>
#include <LoRa.h>

#include "ArduinoJson.h"
#include <Wire.h>

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
int TIME_TO_SLEEP = 5;        /* Time ESP32 will go to sleep (in seconds) */

// Dual Core Operation
TaskHandle_t Task1;
TaskHandle_t Task2;

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2

/*************************** Data Initialization ************************************/
String LoRaData = "";
String configData = "";
String Json_String = "";

byte incoming[]={0};
byte incoming_i2c[] ={0};

void setup() {
  //create a task that will be executed in the Main_Loop() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Main_Loop,   /* Primary function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the Secondary_Loop() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Secondary_Loop,   /* Secondary function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500); 
  
  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  LoRa.begin(866E6);
  LoRa.setSyncWord(0xF3);
  SPIFFS.begin(true);
  
}

void loadConfig(){
  File configFile = SPIFFS.open("/config.txt");
  Json_String = configFile.readString();
  configFile.close();
}

void saveConfig(String Data){
  File configFile = SPIFFS.open("/config.txt", FILE_WRITE);
  configFile.print(Data);
  configFile.close();
}

void Main_Loop(void * pvParameters)
{
  while(true){
    loadConfig();
    
    StaticJsonBuffer<300> JSONBuffer;                         //Memory pool
    JsonObject& parsed = JSONBuffer.parseObject(Json_String); //Parse message
    
    for(JsonObject::iterator it=parsed.begin(); it!=parsed.end(); ++it) // JSON Iterator
    {
        String key = it->key;
        if(key == "ModBus")
        {
          int i = 0;
          if (it->value.is<JsonObject>()) {
            
            JsonObject& value = it->value;
            for(JsonObject::iterator it1=value.begin(); it1!=value.end(); ++it1)
            {
              String key1 = it1->key;
              if(key1 == "baud_rate")
              {
                int value1 = it1->value;
                //Serial.print("SerialBegin:");
                //Serial.println(value1);
                Serial.begin(value1);
              }
              else
              {
                //Serial.print("Key");
                //Serial.println(key1);
                int len = value[key1].size();
                //Serial.print("Len");
                //Serial.println(len);
                byte frame[len]; 
                for(int j=0;j<len;j++)
                {
                  int val = value[key1][j];
                  frame[j] = val;
                  //Serial.print(frame[j]);
                }
                Serial.write(frame,sizeof(frame));  
                delay(50);
                int z = Serial.available();  //Number of Bytes Receiving
                
                while (Serial.available() > 0)
                {
                  incoming[i] =  Serial.read();
                  i++;
                  }delay(50);
              }
              
            }
            Serial.end();
          }
        }
        if(key == "I2C")
        {
          int k = 0;
          if (it->value.is<JsonObject>()) {
            JsonObject& value = it->value;
            for(JsonObject::iterator it1=value.begin(); it1!=value.end(); ++it1)
            {
              String key1 = it1->key;
              if(key1 == "ADDR")
              {
                int value1 = it1->value;
                //Serial.print("WireBegin:");
                //Serial.println(value1);
                Wire.begin();
                Wire.beginTransmission(value1);
              }
              else
              {
                int val =  value[key1];
                Wire.write(val);
                incoming_i2c[k] = Wire.read();
                k++; 
              }
            }
          }
        }
        if(key == "Sleep_Time")
        {
          TIME_TO_SLEEP = it->value;
        }
        
    }

    //Send LoRa packet to receiver
    LoRa.beginPacket();
    LoRa.write(incoming,sizeof(incoming));
    LoRa.write(incoming_i2c,sizeof(incoming_i2c));
    LoRa.endPacket();
    
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); //ESP32 will go to deep sleep for TIME_TO_SLEEP Seconds
    esp_deep_sleep_start();
  }
}

void Secondary_Loop(void * pvParameters)
{
  while(true){
    // try to parse packet
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      // read packet
      while (LoRa.available()) {
      LoRaData = LoRa.readString();
      int len = LoRaData.length();
  
      if(len<10)
      {
        if(LoRaData == "ESP_RESET")
        {
          ESP.restart();  
        }
      }
      else{
      saveConfig(LoRaData);  
        } 
       }
     }
   }
}

void loop() {
  
}
