#include <Arduino.h>

int freq = 1000;
//11 pulser på 10 s --> 66 bpm
int adc1Pin = 32;


void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Hello Everyone");



}

void loop() {
  Serial.print(">");

  delay(10000/freq);
  int adc_value;
  adc_value = analogRead(adc1Pin);
  
  Serial.print(" value: ");
  Serial.println(adc_value);

}