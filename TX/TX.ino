// nRF24L01 TX like master
#include <SPI.h>
#include "RF24.h"
#include <Sleep_n0m1.h>

#define CE 7
#define CS 8

RF24 nRF(CE, CS);

byte adresaPrijimac[]= "prijimac00";
byte adresaVysilac[]= "vysilac00";

void setup() {
  // serial 9600 baud
  Serial.begin(9600);
  // zapnutí komunikace nRF modulu
  nRF.begin();

  // could be RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX,
  nRF.setPALevel(RF24_PA_LOW);

  // set adresses and channel
  nRF.openWritingPipe(adresaVysilac);
  nRF.openReadingPipe(1,adresaPrijimac);
  // start rx
  //nRF.startListening();
}
struct{
 int TH_100; //humidity
 int T_100;  //temperature
 int V_100; //batt voltage 
}datSens;

  int lamp;
  Sleep sleep;  

void loop() {

    ++lamp %= 2;

    
 
    if (!nRF.write( &lamp, sizeof(lamp) )){
      // Serial.println("tx error");
    }
    // turn on receiver
    nRF.startListening();
    // get current time in us
    unsigned long usWait = micros();

    // check timout
    bool timeout = false;
    // wait answer
    while ( ! nRF.available() ){
      // 300 ms max
      if (micros() - usWait > 300000 ){
          timeout = true;
          break;
      }      
    }
    // turn off receiver
    nRF.stopListening();
    // check if not timed out

    if ( !timeout ){
        // get data from receiver
        nRF.read( &datSens, sizeof(datSens) );
        // received
        Serial.print("TX: ");
        Serial.print(lamp);
        Serial.print(", RX: ");
        Serial.print(datSens.TH_100);
        Serial.print(",");        
        Serial.print(datSens.T_100);
        Serial.print(",");
        Serial.println(datSens.V_100);
    }
    // wait printing finished
    delay(100);
    sleep.pwrDownMode(); //set sleep mode
    sleep.sleepDelay(10000); //sleep for: sleepTime
}
