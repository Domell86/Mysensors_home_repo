// **************************************************************//
// AS-0241R 
// ARDUINO 2 Binary inputs, 4 Binary Outputs, 1 PWM output  
// mysensors node 
// SQ9MDD @ 2019
// **************************************************************//


// IO
#define ao1     3     // AO1 led

// radio 
#define MY_RADIO_NRF24
#define MY_DEBUG            // Enable debug prints
#define MY_NODE_ID      8   // <--- !!! SET NODE ADDRESS HERE !!!
#define MY_RF24_CHANNEL 80  // channel from 0 to 125. 76 is default

// library
#include <EEPROM.h>
#include <MySensors.h>

#define CHILD_ID_AO1     1
#define CHILD_ID_BI1     2

MyMessage msgAO1(CHILD_ID_AO1, V_PERCENTAGE);
MyMessage msgBI1(CHILD_ID_BI1, V_TRIPPED);

// zmienne systemowe ********************************************//
boolean bi_state[2] = {false, false};
boolean bi_state_old[2] = {false, false};
unsigned long time_to_send_data = millis();
int ao_state = 0;
int last_minute = 0;
int last_hour = 0;
int set_pwm = 0;
int pwm = 0;
unsigned long time_fade = 0;

// funkcje ******************************************************//
void receiveTime(uint32_t ts){
  setTime(ts);  
}

// setup przy starcie *******************************************//
void setup() {
  // initialize outputs
  pinMode(bo1, OUTPUT);  digitalWrite(bo1,  HIGH);
  pinMode(bo2, OUTPUT);  digitalWrite(bo2,  HIGH);
  pinMode(bo3, OUTPUT);  digitalWrite(bo3,  HIGH);
  pinMode(bo4, OUTPUT);  digitalWrite(bo4,  HIGH);
  pinMode(ao1,OUTPUT);   analogWrite(ao1,0);
  pinMode(bi1,INPUT_PULLUP);
  pinMode(bi2,INPUT_PULLUP);

  // Send the sketch version information to the gateway
  sendSketchInfo("DOM-LAZ", "1.0");

  // Register all sensors to gw 
  // set unique name from node addres and child output name
  // R means Radio transport (if you are using other gateway too)
  char etykieta[] = "       ";
  int addr = MY_NODE_ID;
  sprintf(etykieta,"R%02u.AO1",addr);  present(CHILD_ID_AO1, S_DIMMER, etykieta);
  sprintf(etykieta,"R%02u.BI1",addr);  present(CHILD_ID_BI1, S_DOOR, etykieta);

  // Wyslij domyslne stany wyjsc
  send(msgAO1.set(ao_state,1));

  // wyslij wejscia default
  send(msgBI1.set(bi_state[0],1));
}

void receive(const MyMessage &message){
  switch (message.sensor) {
      case 1:
        int dim_val = message.getInt(); 
        set_pwm = map(dim_val,0,100,0,254);
        delay(100);
        send(msgAO1.set(dim_val));
      break;
  }
}

// petla glowna *************************************************//
void loop(){ 
  if (hour() != last_hour){
    sendHeartbeat();
    last_hour = hour();
  }
    
  if (millis() >= time_fade){
    if(pwm < set_pwm){
       pwm++;
    }
    if(pwm > set_pwm){
       pwm--;
    }    
    analogWrite(ao1,pwm);
    time_fade = millis() + 25; 
  }
}
// **************************************************************//
