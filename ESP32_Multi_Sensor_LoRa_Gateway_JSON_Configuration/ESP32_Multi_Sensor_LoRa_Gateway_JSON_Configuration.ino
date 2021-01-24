// ESP32_Webserver Include files
#include <WiFi.h>
#include <WebSocketServer.h>
#include <ArduinoJson.h>

// BME_280 Sensor Include files
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// LoRa transceivers Include files
#include <SPI.h>
#include <LoRa.h>

// Newtek_Energy_Meter Include files
#include <Newtek_Energy_Meter.h>


WiFiServer server(80);
WebSocketServer webSocketServer;
 
const char* ssid = "yourNetworkName";
const char* password =  "yourNetworkPassword";

//define the pins used by the LoRa transceiver module
#define ss 5
#define rst 14
#define dio0 2

// BME_280 I2C Communication definition
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C

String LoRaData = "";

Energy_Meter energy;

bool BME_280 = false;
bool Energy_Meter = false;

float Voltage = 0.00;
float Current = 0.00;
float Power = 0.00;
float PowerFactor = 0.00;
float Frequency = 0.00;
float Energy = 0.00;

float temperature = 0.00;
float pressure = 0.00;
float altitude = 0.00;
int humidity = 0;

    
void handleReceivedMessage(String message){
 
  StaticJsonBuffer<500> JSONBuffer;                     //Memory pool
  JsonObject& parsed = JSONBuffer.parseObject(message); //Parse message
 
  if (!parsed.success()) {   //Check for errors in parsing
    Serial.println("Parsing failed");
    return;
  }
 
  BME_280 = parsed["BME280"];           //Get Sensor Configuration
  Energy_Meter = parsed["Energy_Meter"]; //Get Sensor Configuration
}


void setup() {
    Serial.begin(9600);
    while(!Serial);    // time to get serial running
    
    //setup LoRa transceiver module
    LoRa.setPins(ss, rst, dio0);
    LoRa.begin(866E6);
    LoRa.setSyncWord(0xF3);

    unsigned status;   
    // default settings
    status = bme.begin();  
    // You can also pass in a Wire library object like &Wire2
    // status = bme.begin(0x76, &Wire2)
}


void loop() { 
    
    WiFi.begin(ssid, password);
    if(WiFi.status() == WL_CONNECTED)
    {
      server.begin();
      delay(100);  
      WiFiClient client = server.available();
 
      if (client.connected() && webSocketServer.handshake(client)) {
        String data;      
        while (client.connected()) {
          data = webSocketServer.getData();
          if (data.length() > 0) {
             handleReceivedMessage(data);
             webSocketServer.sendData(data);
          }
          delay(10); // Delay needed for receiving the data correctly
       }
       delay(100);
      }
      delay(100);
    }
    

    if (BME_280 = true)
    {
      temperature = bme.readTemperature();
      pressure = (bme.readPressure() / 100.0F);
      altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
      humidity = bme.readHumidity();
    }
    else if (BME_280 == false)
    {
      temperature = 0.00;
      pressure = 0.00;
      altitude = 0.00;
      humidity = 0;  
    }
    
    if (Energy_Meter == true)
    {
      Voltage = energy.GetLineVoltage();
      Current = energy.GetLineCurrent();
      Power = energy.GetPower();
      PowerFactor = energy.GetPowerFactor();
      Frequency = energy.GetFrequency();
      Energy = energy.GetImportEnergy();
    }
    else if (Energy_Meter== false)
    {
      Voltage = 0.00;
      Current = 0.00;
      Power = 0.00;
      PowerFactor = 0.00;
      Frequency = 0.00;
      Energy = 0.00;  
    }
    // try to parse packet
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      
      // read packet
      while (LoRa.available()) {
        LoRaData = LoRa.readString();
        Serial.print(LoRaData); 
      }

      int len = LoRaData.length();
      char req[len];
      LoRaData.toCharArray(req, len);
      String Req_Data = "";
      
      for (int i =0; i<=len; i++)
      {
        switch(req[i])
        {
          case 'T':
            Req_Data = Req_Data + "T"+String(temperature)+"T";
            break;
          case 'p':
            Req_Data = Req_Data + "p"+String(pressure)+"p";
            break;
          case 'A':
            Req_Data = Req_Data + "A"+String(altitude)+"A";
            break;
          case 'H':
            Req_Data = Req_Data + "H"+String(humidity)+"H";
            break;
          case 'V':
            Req_Data = Req_Data + "V"+String(Voltage)+"V";
            break;
          case 'I':
            Req_Data = Req_Data + "I"+String(Current)+"I";
            break;
          case 'P':
            Req_Data = Req_Data + "P"+String(Power)+"P";
            break;
          case 'F':
            Req_Data = Req_Data + "F"+String(Frequency)+"F";
            break;
          case 'Φ':
            Req_Data = Req_Data + "Φ"+String(PowerFactor)+"Φ";
            break;
          case 'E':
            Req_Data = Req_Data + "E"+String(Energy)+"E";
            break;
        }
      }
      //Send LoRa packet to receiver
      LoRa.beginPacket();
      LoRa.print(Req_Data);
      LoRa.endPacket();
    }
}
