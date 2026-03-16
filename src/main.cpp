#include <Arduino.h>
#include "driver/gpio.h"

int freq = 1000;
//12 pulser på 10 s --> 72 bpm
int adc1Pin = 32;
int ledPin = 14; //fixa led
int max_threshold = 800;
int min_threshold = 500;
int adc_top_value = 0;
bool at_top = false;
int pulse = 0;


void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Hello Everyone");

}

void loop() {

  delay(10000/freq);
  printf("max: %d \n", max_threshold);
  printf("min: %d \n", min_threshold);
  int adc_value;
  adc_value = analogRead(adc1Pin);

  if (adc_value < 3500 && adc_value > adc_top_value) {
    adc_top_value = adc_value;
  } 
  max_threshold = adc_top_value * 0.90;
  min_threshold = adc_top_value * 0.85;


  if (adc_value >= max_threshold) {
    //tänd led
    if(at_top == false) {
      at_top = true;
      pulse++;
    }
  }
  if (adc_value <= min_threshold) {
    //släck led
    at_top = false;
    //pulse++;
  }

  printf("antal pulser: %d \n", pulse);

  Serial.print(">");
  Serial.print(">");
  Serial.print("value:");
  Serial.print(adc_value);
  Serial.println();
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