#include <Arduino.h>
#include "circularBuffer.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

int freq = 1000;
volatile int timer_counter = 0;

int adc1Pin = 32;
int ledPin = 14; //fixa led
// int SDApin = 21
// int SCLpin = 22;

int max_threshold = 800;
int min_threshold = 500;
volatile int peak = 0;
bool at_top = false;
volatile int pulses = 0;
bool pulseDetected = false;

volatile int bpm = 10;
int lastTime = 0;

hw_timer_t *timer = NULL;

struct circularBuffer intervals; 

struct circularBuffer normalization;
int offset = 0;
 

#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

Adafruit_SSD1306 oled(128, 32, &Wire, -1);

int adc_value;

int Signal;                // holds the incoming raw data. Signal value can range from 0-1024

int x=0;                   // current position of the cursor
int y=0;
int lastx=0;                // last position of the cursor
int lasty=0;



void checkpulseNInterval(int filtered_value);

void IRAM_ATTR sampleCallback() {
  
  int adc_value;
  adc_value = analogRead(adc1Pin);

  addElement(&normalization,adc_value);

  offset = getAverage(&normalization);

  int normalized_value = adc_value - offset;

  //TODO sofia normalisera (4.8), skapa en circular buffer med 300 värden och räkna offseten 

 



  //TODO tsm filtrera, high & low, ta koden från tidigare labbar, använd matlab för att hitta koefficienterna

  int filtered_value = normalized_value;

  checkpulseNInterval(filtered_value);

  timer_counter++;
}

  //I setup sker följande: 
  //Startar serial monitorn
  //pinModes
  //init av buffers
  //timers/callback 
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Hello Everyone");

  pinMode(ledPin, OUTPUT);

  // initialize OLED display with address 0x3C for 128x64
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  delay(2000);         // wait for initializing
  oled.clearDisplay(); // clear display

  oled.setTextSize(1);          // text size
  oled.setTextColor(WHITE);     // text color
  oled.setCursor(0, 0);        // position to display
  oled.println("Hello World!"); // text to display
  oled.display();               // show on OLED
  delay(2000);
  oled.clearDisplay();        // clear display


  int* buf_data1 = (int*) malloc(20 * sizeof(int));
  initCircularBuffer(&intervals,buf_data1, 20);

  int* buf_data2 = (int*) malloc(300 * sizeof(int));
  initCircularBuffer(&normalization,buf_data2, 300);

  timer = timerBegin(0,80,true);
  timerAttachInterrupt(timer,&sampleCallback,true);
  timerAlarmWrite(timer, freq, true);
  timerAlarmEnable(timer);
}

  //Metoden kollar om en puls skett och isåfall sätter pulsedetected = true
  //Uppdatering av threshold sker också här 
void checkpulseNInterval(int filtered_value) {
  //printf("peak: %d \n", peak_threshold);
  //printf("min: %d \n", min_threshold);
  

  if (filtered_value > peak) {
    peak = filtered_value;
  } 
 
  max_threshold = peak * 0.75; 
  min_threshold = peak * 0.50;

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

  int forOLED = adc_value-2000;
  Serial.println(forOLED);                    // Send the Signal value to Serial Plotter.

  if(x>SCREEN_WIDTH - 1){                    // reset the screen when cursor reaches the border of the LED screen
      oled.clearDisplay();
      x=0;
      lastx=x;
  }

  //These values need to be determined from your signal dynamically
  int lower_bound = 0;                      //minimum signal values
  int upper_bound = 1000;                   //Maximum signal values

  float conversion = (upper_bound-lower_bound)/float(SCREEN_HEIGHT);  // a variable is needed fit signal range received by ADC into the screen height

  y = SCREEN_HEIGHT-((forOLED-lower_bound)/conversion)-1;              // strength of the signal in the screen coordinates
  oled.writeLine(lastx,lasty,x,y,WHITE);                              // write a line between previous and the current cursor positions
  lasty=y;
  lastx=x;
  x++;
  oled.display();

  delay(10);

    //Denna del räknar ut intervallet mellan varje puls
  if(pulseDetected) {

    int timeNow = millis();
    int interval = timeNow - lastTime; 
    lastTime = timeNow; 
 
    addElement(&intervals, interval);
    pulseDetected = false;

  }
    //Denna del räknar ut bpm genom att ta average values (metod i circBuff) av varje intervall mellan pulser
    //Detta sker var 20e sekund då sampling sker var 1 ms (1000 Hz)
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