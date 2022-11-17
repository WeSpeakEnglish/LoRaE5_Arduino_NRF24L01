// nRF24L01 přijímač

// připojení knihoven
#include <Arduino.h>
#include <SPI.h>
#include "RF24.h"
#include <Sleep_n0m1.h>
#include <Wire.h>
#include <GyverHTU21D.h>

// NRF24L01 pins  connected to...
#define CE 7
#define CS 8
// inicialize NRF24L01 pins  connected to...
RF24 nRF(CE, CS);
// nastavení adres pro přijímač a vysílač,
// musí být nastaveny stejně v obou programech!
byte adresaPrijimac[]= "prijimac00";
byte adresaVysilac[]= "vysilac00";

Sleep sleep;

GyverHTU21D htu;

struct{
 int TH_100; //humidity
 int T_100;  //temperature
 int V_100; //batt voltage 
}datSens;

  //int lamp;
  uint8_t lamp[32];
  volatile int count = 0;
  // Timer 1 interrupt service routine (ISR)
ISR(TIMER1_COMPA_vect)
{
   // Sample voltage on pin A0
   count++;
}

void setup() {
  datSens.TH_100 = 5000; //50%
  datSens.T_100 = 2000; //20 deg C
  datSens.V_100 = 1200; //12V 
  // komunikace přes sériovou linku rychlostí 9600 baud
  Serial.begin(9600);
  // zapnutí komunikace nRF modulu
  nRF.begin();
  // nastavení výkonu nRF modulu,
  // možnosti jsou RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX,
  // pro HIGH a MAX je nutný externí 3,3V zdroj
  nRF.setPALevel(RF24_PA_LOW);
  // nastavení zapisovacího a čtecího kanálu
  nRF.openWritingPipe(adresaPrijimac);
  nRF.openReadingPipe(1,adresaVysilac);
  // začátek příjmu dat
  nRF.startListening();

  if (!htu.begin()) Serial.println(F("HTU21D error"));
  delay(100);
  char debugA[1000];
  nRF.sprintfPrettyDetails(debugA);
  Serial.print(debugA);
  cli(); // disable interrupts during setup
  // Configure Timer 1 interrupt
  // F_clock = 16e6 Hz, prescaler = 64, Fs = 1 Hz
  TCCR1A = 0;
  TCCR1B = 1<<WGM12 | 0<<CS12 | 1<<CS11 | 1<<CS10;
  TCNT1 = 0;          // reset Timer 1 counter
  // OCR1A = ((F_clock / prescaler) / Fs) - 1 = 2499
  OCR1A = 24999;       // Set sampling frequency Fs = 10 Hz
  TIMSK1 = 1<<OCIE1A; // Enable Timer 1 interrupt
 
  sei(); // re-enable interrupts

}



 
void loop() {
  // proměnné pro příjem a odezvu
  if(count > 9){
    datSens.T_100= (int)(100.0 * htu.getTemperatureWait());
    datSens.TH_100= (int)(100.0 * htu.getHumidityWait());
    datSens.V_100 = analogRead(0);
    count = 0;
  }

  if( nRF.available()){
    // čekání na příjem dat
    while (nRF.available()) {
      // v případě příjmu dat se provede zápis
      // do proměnné prijem
    ////  nRF.read( &lamp, sizeof(lamp) );
    nRF.read( lamp, sizeof(lamp) );
    }
    // vytisknutí přijatých dat na sériovou linku
    switch( lamp[0] ) {
      case 0:
        Serial.println("LAMP OFF ");
        break;
      case 1:  
        Serial.println("LAMP ON ");
        break;
    }
    
    // ukončení příjmu dat
    nRF.stopListening();
    // odeslání odezvy 
    nRF.write( &datSens, sizeof(datSens) );     
    // přepnutí do příjmu dat pro další komunikaci
    nRF.startListening();
    // vytištění odezvy po sériové lince     
   // Serial.print(", odezva: ");
   // Serial.println(odezva);  
  }
}
