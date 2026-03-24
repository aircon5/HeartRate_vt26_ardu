
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "circularBuffer.h"
#include "pro-highpass-fdacoefs.h"
#include "pro-lowpass-fdacoefs.h"

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
int counter;

bool pulseDetected = false;
bool valueAvailable = false;

volatile int bpm = 10;
int lastTime = 0;

hw_timer_t *timer = NULL;

struct circularBuffer intervals; 

struct circularBuffer normalization;
int offset = 0;

static float xbuff_l[NL_L] = {0};
static float ybuff_l[NL_L] = {0};
static float xbuff_h[NL_H] = {0};
static float ybuff_h[NL_H] = {0};
 

#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

Adafruit_SSD1306 oled(128, 32, &Wire, -1);

int adc_value;
int normalized_value;
int filtered_value;

int x=0;                   // current position of the cursor
int y=0;
int lastx=0;                // last position of the cursor
int lasty=0;


void checkpulseNInterval(int filtered_value);
float filter(int normalized_value);

  //Callback function som hämtar adc-value, normalisera, och flaggar att value är available att
  //Finns öven en timer_counter
void IRAM_ATTR sampleCallback() {
  
  adc_value = analogRead(adc1Pin);

  addElement(&normalization,adc_value);

  offset = getAverage(&normalization);

  normalized_value = adc_value - offset;
  
  valueAvailable = true;

  timer_counter++;
}

  //I setup sker följande: 
  //Startar serial monitorn
  //pinModes
  //display setup
  //init av buffers
  //timers/callback 
void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(ledPin, OUTPUT);

  // initialize OLED display with address 0x3C for 128x64
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  delay(2000);         // wait for initializing
  oled.clearDisplay(); // clear display

  int* buf_data1 = (int*) malloc(10 * sizeof(int));
  initCircularBuffer(&intervals,buf_data1, 10);

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

  if (filtered_value > peak) {
    peak = filtered_value;
  } 
  peak *= 0.99;

  max_threshold = peak * 0.75; 
  min_threshold = peak * 0.50; 

  if(max_threshold <= 300 && min_threshold <= 300) 
  {
    max_threshold = 300;
    min_threshold = 300;
  }


  if (filtered_value >= (int)max_threshold) {
    //tänd led
    if(at_top == false) {
      at_top = true;
      pulses++;
      pulseDetected = true; 
    }
  }
  if (filtered_value <= (int)min_threshold) {
    //släck led
    at_top = false;
  }

}

  //
float filter(int normalized_value) 
{
  
    int invalue = normalized_value;
    for(int k = NL_L-2; k>=0; k--) {
        xbuff_l[k+1]=xbuff_l[k];
    }
    xbuff_l[0] = (float)invalue;

    float sum1 = 0;
    for(int k = 0; k < NL_L; k++) {
        sum1 += NUM_L[k] * xbuff_l[k];
    }

    float sum2 = 0;
    for(int k = 1; k < NL_L; k++) {
        sum2 -= DEN_L[k] * ybuff_l[k];
    }

    float filter_sum = sum1 - sum2;

    for(int k = NL_L-2; k>=1; k--) {
        ybuff_l[k+1] = ybuff_l[k];
    }
    ybuff_l[1] = filter_sum;



    invalue = filter_sum;
    for(int k = NL_H-2; k>=0; k--) {
        xbuff_h[k+1]=xbuff_h[k];
    }
    xbuff_h[0] = (float)invalue;

     sum1 = 0;
    for(int k = 0; k < NL_H; k++) {
        sum1 += NUM_H[k] * xbuff_h[k];
    }

     sum2 = 0;
    for(int k = 1; k < NL_H; k++) {
        sum2 -= DEN_H[k] * ybuff_h[k];
    }

     filter_sum = sum1 - sum2;

    for(int k = NL_H-2; k>=1; k--) {
        ybuff_h[k+1] = ybuff_h[k];
    }
    ybuff_h[1] = filter_sum;

    float outvalue = filter_sum;
  return outvalue;
}


void loop() {
  int scaled_value = filtered_value; 
  Serial.println(scaled_value);                    // Send the Signal value to Serial Plotter.

  if(x>SCREEN_WIDTH - 1){                    // reset the screen when cursor reaches the border of the LED screen
      oled.clearDisplay();
      x=0;
      lastx=x;
  }

  //These values need to be determined from your signal dynamically
  int lower_bound = -700;                      //minimum signal values
  int upper_bound = 1500;                   //Maximum signal values
  //int lower_bound = -200;                      //minimum signal values
  //int upper_bound = 400;                   //Maximum signal values

  float conversion = (upper_bound-lower_bound)/float(SCREEN_HEIGHT);  // a variable is needed fit signal range received by ADC into the screen height
  y = SCREEN_HEIGHT-((scaled_value-lower_bound)/conversion)-1;              // strength of the signal in the screen coordinates
  if(y<12) y = 12;
  if(y>31) y = 31;
  oled.writeLine(lastx,lasty,x,y,WHITE);                              // write a line between previous and the current cursor positions
  lasty=y;
  lastx=x;
  x++;
  oled.display();

  oled.setTextSize(1);          // text size
  oled.setTextColor(WHITE);     // text color
  oled.setCursor(0, 0);        // position to display
  oled.printf("BPM: %d",  bpm); // text to display
  oled.display();               // show on OLED

  delay(10);

    //Denna del räknar ut intervallet mellan varje puls
  if(pulseDetected) {
    int timeNow = millis();
    int interval = timeNow - lastTime; 
    lastTime = timeNow; 
 
    if (interval > 300 && interval < 2000) {
      addElement(&intervals, interval);
    }
    pulseDetected = false;
    
  } 
  int timeNow = millis();
  if (timeNow - lastTime > 2000) {
    bpm = 0;
  }
  
    //Denna del räknar ut bpm genom att ta average values (metod i circBuff) av varje intervall mellan pulser
    //Detta sker var 10e sekund då sampling sker var 1 ms (1000 Hz)
  if(timer_counter - 10000 >= 0) {
    delay(10);
    timer_counter = 0;
    float average = (float)getAverage(&intervals);
    if(average > 0) {
      bpm = (int) 60000 / average;
    } else {
      bpm = 0;
    }

  }

  if(valueAvailable) {
    filtered_value = (int) filter(normalized_value);

    checkpulseNInterval(filtered_value);
    valueAvailable = false;
  }

  
  //printf("Timer_counter %d \n", timer_counter);
  printf("BPM %d \n", bpm);
  printf("pulses %d \n", pulses);
  printf("max_t %d \n", max_threshold);
  printf("min_t %d \n", min_threshold);

  Serial.print(">");
  Serial.print("raw_value:");
  Serial.print(adc_value);
  Serial.print(",");
  Serial.print("norm_value:");
  Serial.print(normalized_value);
  Serial.print(",");
  Serial.print("filt_value:");
  Serial.print(filtered_value);
  Serial.print(",");
  

}