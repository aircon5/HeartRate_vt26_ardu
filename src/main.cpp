#include <Arduino.h>
#include "driver/gpio.h"
#include "circularBuffer.h"

int freq = 1000;
//12 pulser på 10 s --> 72 bpm
int adc1Pin = 32;
int ledPin = 14; //fixa led
int max_threshold = 800;
int min_threshold = 500;
volatile int peak = 0;
bool at_top = false;
volatile int pulses = 0;
bool pulseDetected = false;
volatile int timer_counter = 0;

volatile int bpm = 10;
int lastTime = 0;
hw_timer_t *timer = NULL;

struct circularBuffer intervals; 


void checkpulseNInterval(int filtered_value);

void IRAM_ATTR sampleCallback() {
  
  int adc_value;
  adc_value = analogRead(adc1Pin);

  //TODO sofia normalisera, skapa en circular buffer med summan av 300 värden och räkna offseten 

  //TODO tsm filtrera, high & low, ta koden från tidigare labbar, använd matlab för att hitta koefficienterna

  //TODO annika calculate bpm med tidsintervall mellan varje peak 

  int filtered_value = adc_value;

  checkpulseNInterval(filtered_value);

  timer_counter++;
}


void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Hello Everyone");

  int* buf_data = (int*) malloc(20 * sizeof(int));
  initCircularBuffer(&intervals,buf_data, 20);

  timer = timerBegin(0,80,true);
  timerAttachInterrupt(timer,&sampleCallback,true);
  timerAlarmWrite(timer, freq, true);
  timerAlarmEnable(timer);
}

//metoden ska kolla när det är en puls och räkna tiden mellan det
void checkpulseNInterval(int filtered_value) {
  //printf("peak: %d \n", peak_threshold);
  //printf("min: %d \n", min_threshold);
  

  if (filtered_value > peak) {
    peak = filtered_value;
  } 


  //TODO ändra *0._ senare efter normalisering gjorts
  max_threshold = peak * 0.90; 
  min_threshold = peak * 0.85;


  if (filtered_value >= max_threshold) {
    //tänd led
    if(at_top == false) {
      at_top = true;
      pulses++;
      pulseDetected = true; 
      
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
  delay(10);
  if(pulseDetected) {
    int timeNow = millis();
    int interval = timeNow - lastTime; 
    lastTime = timeNow; 
 
    addElement(&intervals, interval);
    //TODO ta tiden istället mellan varje pulse och lägg in i array
    //  reset tiden
    pulseDetected = false;
  }

  if(timer_counter - 20000 >= 0) {
    delay(10);
    timer_counter = 0;
    bpm = (int) 60000 / (float)getAverage(&intervals) ;

  }
  
  delay(10); 
  
  //printf("Timer_counter %d \n", timer_counter);
  printf("BPM %d \n", bpm);
  printf("pulses %d \n", pulses);
  printf("max_t %d \n", max_threshold);
  printf("min_t %d \n", min_threshold);
  

}