// **************************************************************//
// AS-310RRDS code name DOMELL
// ARDUINO One relay aand three temp sensors
//
// SQ9MDD @ 2019
// **************************************************************//
// CHANGELOG
// 2021.01.18 initial commit test


#define ONE_WIRE_BUS 7
#define RELAY 3

// radio 
#define MY_RADIO_NRF24
#define MY_DEBUG                                    // Enable debug prints
//#define MY_GATEWAY_SERIAL                         // Enable GW serial
#define MY_REPEATER_FEATURE
#define MY_NODE_ID      55                          // <--- !!! SET NODE ADDRESS HERE !!!
#define MY_RF24_CHANNEL 80                          // channel from 0 to 125. 76 is default

#include <MySensors.h>
#include <TimeLib.h>                                // https://github.com/PaulStoffregen/Time
#include <DallasTemperature.h>                      // biblioteka do komunikacji z czujnikami DS18B20
#include <OneWire.h>                                // biblioteka do komunikacji z czujnikami one wire

#define CHILD_ID_AI1     1                          // AI1 T1
#define CHILD_ID_AI2     2                          // AI2 T2
#define CHILD_ID_AI3     3                          // AI3 T3
#define CHILD_ID_BO1     4                          // AI4 RELAY

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

MyMessage msgAI1(CHILD_ID_AI1, V_TEMP);
MyMessage msgAI2(CHILD_ID_AI2, V_TEMP);
MyMessage msgAI3(CHILD_ID_AI3, V_TEMP);
MyMessage msgBO1(CHILD_ID_BO1, V_STATUS);

boolean bo_state = false;

int last_minute = 0;
int last_second = 0;

float temp = 0.0;
float temp_avg = 0.0;

void receiveTime(uint32_t ts){
  setTime(ts);  
}

void BO_SET(int pin) {
  int pin_out = 0;
  switch (pin){
    case 4:
      bo_state = true;  
      digitalWrite(RELAY, LOW);
      send(msgBO1.set(bo_state));
    break;
  } 
}

void BO_RESET(int pin) {
  int pin_out = 0;
  switch (pin){
    case 4:  
      bo_state = false;
      digitalWrite(RELAY, HIGH);
      send(msgBO1.set(bo_state));
    break;
  }
}

void receive(const MyMessage &message){
  switch (message.sensor) {     
      case 4:
        if (message.getBool() == true){ BO_SET(4); } else{ BO_RESET(4); }
      break;      
  }
}

void presentation(){
  char etykieta[] = "       ";
  int addr = MY_NODE_ID;   
  sendSketchInfo("AS-310RRDS", "1.0");
  sprintf(etykieta,"R%02u.AI1",addr);     present(CHILD_ID_AI1, S_TEMP, etykieta);
  sprintf(etykieta,"R%02u.AI2",addr);     present(CHILD_ID_AI2, S_TEMP, etykieta);
  sprintf(etykieta,"R%02u.AI3",addr);     present(CHILD_ID_AI3, S_TEMP, etykieta); 
  sprintf(etykieta,"R%02u.BO1",addr);     present(CHILD_ID_BO1, S_BINARY, etykieta);
}

void setup(){  
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH);
  delay(1000);  
  requestTime();
  sensors.requestTemperatures();
  int16_t conversionTime = sensors.millisToWaitForConversion(sensors.getResolution());
  send(msgAI1.set(sensors.getTempCByIndex(0), 1)); 
  send(msgAI2.set(sensors.getTempCByIndex(1), 1)); 
  send(msgAI3.set(sensors.getTempCByIndex(2), 1));
  send(msgBO1.set(0));
}

void loop(){
  if (second() != last_second){ 
    last_second = second(); 
  }
  
  if (minute() != last_minute){   
    sensors.requestTemperatures();  
    int16_t conversionTime = sensors.millisToWaitForConversion(sensors.getResolution());
    send(msgAI1.set(sensors.getTempCByIndex(0), 1)); 
    send(msgAI2.set(sensors.getTempCByIndex(1), 1)); 
    send(msgAI3.set(sensors.getTempCByIndex(2), 1));
    send(msgBO1.set(bo_state));    
    last_minute = minute();
  }
}
