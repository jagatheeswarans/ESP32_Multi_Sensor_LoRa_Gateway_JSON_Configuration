#include "ArduinoJson.h"
#include <Wire.h>
 
void setup() {
 
  Serial.begin(115200);
  Serial.println();
 
}
byte incoming[]={0};
byte incoming_i2c[] ={0};

void loop() {
 
  Serial.println("Parsing start: ");
 
  char JSONMessage[] = "{\"ModBus\":{\"baud_rate\":\"9600\",\"Voltage\":[1,4,0,0,0,2,113,203],\"Current\":[1,4,0,6,0,2,145,202]}}"; //Original message
 
  StaticJsonBuffer<300> JSONBuffer;                         //Memory pool
  JsonObject& parsed = JSONBuffer.parseObject(JSONMessage); //Parse message
 
  if (!parsed.success()) {   //Check for errors in parsing
 
    Serial.println("Parsing failed");
    delay(2000);
    return;
 
  }

for(JsonObject::iterator it=parsed.begin(); it!=parsed.end(); ++it)
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
            Serial.print("SerialBegin:");
            Serial.println(value1);
          }
          else
          {
            Serial.print("Key");
            Serial.println(key1);
            int len = value[key1].size();
            Serial.print("Len");
            Serial.println(len);
            byte frame[len]; 
            for(int j=0;j<len;j++)
            {
              int val = value[key1][j];
              frame[j] = val;
              Serial.print(frame[j]);
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
            Serial.print("WireBegin:");
            Serial.println(value1);
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
    
}
  Serial.println();
  delay(2000);
}
