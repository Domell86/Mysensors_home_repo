// **************************************************************//
// AS-0241R 
// ARDUINO 2 Binary inputs, 4 Binary Outputs, 1 PWM output  
// mysensors node 
// SQ9MDD @ 2019
// **************************************************************//

// radio 
#define MY_RADIO_NRF24
#define MY_REPEATER_FEATURE
#define MY_DEBUG            // Enable debug prints
#define MY_NODE_ID      1   // <--- !!! SET NODE ADDRESS HERE !!!
#define MY_RF24_CHANNEL 80  // channel from 0 to 125. 76 is default

// IO
#define ao1     3     // AO1 led
#define bi1     4     // BI1 czujnik ruchu

// library
#include <EEPROM.h>
#include <MySensors.h>

#define CHILD_ID_AO1     1
#define CHILD_ID_BI1     2

MyMessage msgAO1(CHILD_ID_AO1, V_PERCENTAGE);
MyMessage msgBI1(CHILD_ID_BI1, V_TRIPPED);

// zmienne systemowe *********************************************//
boolean bi_state[1] = {false};                                    // tablica zmiennych dla stanów wejść
boolean bi_state_old[1] = {false};                                // tablica zmiennych dla zapisanych poprzednio stanów wejść
int set_pwm = 0;                                                  // zadana wartość PWM
int pwm = 0;                                                      // aktualna wartość PWM
unsigned long time_fade = 0;                                      // zmienna pomocnicza do obsługi ściemniania
unsigned long time_to_back = 0;                                   // zmienna pomocnicza do obsługi czasów wysyłki stanu BI
unsigned long bi_interval = 5000;                                 // interwał wysyłki danych po przejśćiu BI na false
unsigned long fade_interval = 25;                                 // czas w mS pomiędzy zmianami wysterowania PWM

// funkcje *******************************************************//
void presentation(){                                              // funkcja prezentacji węzła
  sendSketchInfo("DOM-LAZ", "1.2");                               // Send the sketch version information to the gateway
  char etykieta[] = "       ";                                    // zmienna pomocnicza
  int addr = MY_NODE_ID;                                          // pobieram adres ze stałej  
  sprintf(etykieta,"R%02u.AO1",addr);                             // przygotowuję prezentację dla AO1
  present(CHILD_ID_AO1, S_DIMMER, etykieta);                      // wysyłam prezentację AO1
  sprintf(etykieta,"R%02u.BI1",addr);                             // przygotowuję prezentację dla BI1
  present(CHILD_ID_BI1, S_MOTION, etykieta);                        // wysyłam prezentację dla BI1
}

void receive(const MyMessage &message){                           // jeśli przyszła komenda po radiu
  switch (message.sensor) {                                       // sprawdź którego sensora dotyczy
      case 1:                                                     // jeśli sensor nr 1 czyli AO1
        int dim_val = message.getInt();                           // pobierz przesłaną wartość (0% - 100% wysterowania)
        set_pwm = map(dim_val,0,100,0,254);                       // przemapuj wartość % na wartość PWM (0-255)
        delay(100);                                               // odczekaj 100mS
        send(msgAO1.set(dim_val));                                // odeślij po radiu że zrobione
      break;
  }
}

// setup przy starcie ********************************************//
void setup() {
  pinMode(bi1,INPUT);                                             // skonfiguruj wejście dla czujnika ruchu
  send(msgAO1.set(0.0,1));                                        // Wyslij domyslny stan wyjscia po radiu
  send(msgBI1.set(bi_state[0]));                                  // wyslij domyślny stan wejscia
}

// petla glowna **************************************************//
void loop(){ 

  if(digitalRead(bi1) == HIGH){                                   // sprawdź stan czujki i jeśli jest wzbudzona
    delay(50);                                                    // odczekaj 50mS by wykluczyć drgania styków
    if(digitalRead(bi1) == HIGH){                                 // sprawdź stan czujki raz jeszcze i jeśli nadal jest wzbudzona
      bi_state[0] = true;                                         // ustaw wartość zmiennej na true  
    }
  } else{                                                         // jęsli czujka nie jest wzbudzona
    bi_state[0] = false;                                          // ustaw wartość zmiennej na false
  }
  
  if(bi_state_old[0] != bi_state[0]){                             // jesli zmienil sie stan czujki od poprzednio zapisanego stanu
    if(bi_state[0] == true){                                      // i stan jest czujka wzbudzona
      send(msgBI1.set(bi_state[0]));                              // wyslij do domoticza
      bi_state_old[0] = bi_state[0];                              // zapisz ostatni stan 
      time_to_back = millis() + bi_interval;                      // ustaw czas ewentualnego powrotu
    } else{                                                       // jeśli czujka nie jest wzbudzona
      if(millis() >= time_to_back){                               // i minął czas do wyłączenia
        send(msgBI1.set(bi_state[0]));                            // wyślij zmianę do domoticza
        bi_state_old[0] = bi_state[0];                            // zapisz ostatni stan
      }      
    }
  }
  
  if(bi_state_old[0] == bi_state[0] && bi_state[0] == true){      // jesli stan sie nie zmienił i czujka jest wzbudzona
    time_to_back = millis() + bi_interval;                        // ustaw czas ewentualnego powrotu tak by odliczac czas od zejscia sygnału z czujki
  }
  
  if (millis() >= time_fade){                                     // jeśli bieżący czas wiekszy niż czas do śćiemnienia o 1% to wykonaj
    if(pwm < set_pwm){                                            // jesli bieżący PWM jest mniejszy niż zadany PWM
       pwm++;                                                     // dodaj jeden
    }
    if(pwm > set_pwm){                                            // jeśli bieżący PWM jest wiekszy niż zadany PWM
       pwm--;                                                     // odejmij jeden
    }    
    analogWrite(ao1,pwm);                                         // ustaw wysterowanie wyjścia na bieżący PWM
    time_fade = millis() + fade_interval;                                    // ustaw czas wykonania następnego kroku za 25mS
  }
}
// **************************************************************//
