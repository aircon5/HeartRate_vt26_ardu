#include <Arduino.h>
#include "driver/gpio.h"

int freq = 1000;
//12 pulser på 10 s --> 72 bpm
int adc1Pin = 32;
int ledPin = 14; //fixa led
int max_threshold = 800;
int min_threshold = 500;
volatile int peak = 0;
bool at_top = false;
volatile int pulse = 0;
volatile int bpm = 0;
volatile int timer_counter = 0;


hw_timer_t *timer = NULL;

void checkPulse(int filtered_value);

void IRAM_ATTR sampleCallback() {
  
  int adc_value;
  adc_value = analogRead(adc1Pin);

  //TODO sofia normalisera, skapa en circular buffer med summan av 300 värden och räkna offseten 

  //TODO tsm filtrera, high & low, ta koden från tidigare labbar, använd matlab för att hitta koefficienterna

  //TODO annika calculate bpm med tidsintervall mellan varje peak 

  int filtered_value = adc_value;

  checkPulse(filtered_value);

  

  timer_counter++;
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Hello Everyone");
  
  timer = timerBegin(0,80,true);
  timerAttachInterrupt(timer,&sampleCallback,true);
  timerAlarmWrite(timer, freq, true);
  timerAlarmEnable(timer);
}

//metoden ska kolla när det är en puls och räkna tiden mellan det
void checkPulse(int filtered_value) {
  //printf("peak: %d \n", peak_threshold);
  //printf("min: %d \n", min_threshold);
  

  if (filtered_value > peak) {
    peak = filtered_value;
  } 
  max_threshold = peak * 0.90;
  min_threshold = peak * 0.75;


  if (filtered_value >= max_threshold) {
    //tänd led
    if(at_top == false) {
      at_top = true;
      pulse++;
      //TODO ta tiden istället mellan varje pulse och lägg in i array
      //  reset tiden
    }
  }
  if (filtered_value <= min_threshold) {
    //släck led
    at_top = false;
  }

  //printf("antal pulser: %d \n", pulse);
/*
  Serial.print(">");
  Serial.print(">");
  Serial.print("value:");
  Serial.print(filtered_value);
  Serial.println();
  */


}


void loop() {

  delay(10000/freq);
  printf("BPM %d \n", bpm);
  printf("pulses %d \n", pulse);
  printf("max_t %d \n", max_threshold);
  printf("min_t %d \n", min_threshold);


  
  
/*
  Serial.print(">");
  Serial.print("max:");
  Serial.print(max_threshold);
  Serial.println();
  Serial.print(">");
  Serial.print("min:");
  Serial.print(min_threshold);
  Serial.println();
*/

}