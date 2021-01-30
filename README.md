# ESP32_Multi_Sensor_LoRa_Gateway_JSON_Configuration

This code helps to connect ESP32 with multiple sensor and it can transfer the sensor data through LoRa WAN based on the requests. And we can also configure the Sensors using JSON file.

JSON Config File:

{"ModBus":{"baud_rate":"9600","Voltage":[1,4,0,0,0,2,113,203],"Current":[1,4,0,6,0,2,145,202]},"I2C":{"ADDR":"119","Temperature":"250","Pressure":"247","Humidity":"253"},"Sleep_Time":"5"}
