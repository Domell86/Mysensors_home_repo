// **************************************************************//
// AS-0241R 
// ARDUINO 2 Binary inputs, 4 Binary Outputs, 1 PWM output  
// mysensors node 
// SQ9MDD @ 2019
// **************************************************************//


// IO
#define ao1     3     // AO1 led
#define bi1     4     // BI1 czujnik ruchu

// radio 
#define MY_RADIO_NRF24
#define MY_REPEATER_FEATURE
#define MY_DEBUG            // Enable debug prints
#define MY_NODE_ID      1   // <--- !!! SET NODE ADDRESS HERE !!!
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
unsigned long time_to_back = 0;

// funkcje ******************************************************//
void presentation(){
  // Send the sketch version information to the gateway
  sendSketchInfo("DOM-LAZ", "1.0");
    
  char etykieta[] = "       ";
  int addr = MY_NODE_ID;
  sprintf(etykieta,"R%02u.AO1",addr);  present(CHILD_ID_AO1, S_DIMMER, etykieta);
  sprintf(etykieta,"R%02u.BI1",addr);  present(CHILD_ID_BI1, S_DOOR, etykieta);  
}

// setup przy starcie *******************************************//
void setup() {
  // initialize outputs
  pinMode(bi1,INPUT);

  // Wyslij domyslne stany wyjsc
  send(msgAO1.set(ao_state,1));

  // wyslij wejscia default
  send(msgBI1.set(bi_state[0]));
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
  if(bi_state_old[0] != bi_state[0]){
    if(bi_state[0] == true){
      send(msgBI1.set(bi_state[0]));
      bi_state_old[0] = bi_state[0];
      time_to_back = millis() + 10000;
    } else{
      if(millis() >= time_to_back){
        send(msgBI1.set(bi_state[0]));
        bi_state_old[0] = bi_state[0];        
        time_to_back = millis() + 10000;  
      }
      
    }
  }
  
  if(digitalRead(bi1) == HIGH){
    delay(50);
    if(digitalRead(bi1) == HIGH){
      bi_state[0] = true;     
    }
  } else{
    bi_state[0] = false;  
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
