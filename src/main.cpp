
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

#define M 3
#define N 3
#define ORDER 3

void checkpulseNInterval(int filtered_value);
float filter(int normalized_value);

  //Callback function som hämtar adc-value, normalisera, och flaggar att value är available att
  //Finns öven en timer_counter
void IRAM_ATTR sampleCallback() {
  
  adc_value = analogRead(adc1Pin);
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

  if(max_threshold <= 300 && min_threshold <= 200) 
  {
    max_threshold = 300;
    min_threshold = 200;
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

  
float filter(int norm_value) 
{
  //Low pass filter
  static float xbuff_low[M+1] = {0};
  static float b_low[M+1] = {
      3.756838019751e-06,1.127051405925e-05,1.127051405925e-05,3.756838019751e-06
  };
  static float ybuff_low[N+1] = {0};
  static float a_low[N+1] = {
      1,    2.93717072845,    -2.876299723479,  0.9390989403253
  };

  int invalue_low = norm_value;
  for(int k = M-1; k>=0; k--) {
    xbuff_low[k+1]=xbuff_low[k];
  }
  xbuff_low[0] = (float)invalue_low;

  float sum1_low = 0;
  for(int k = 0; k <= M; k++) {
    sum1_low += b_low[k] * xbuff_low[k];
  }

  float sum2_low = 0;
  for(int k = 1; k <= N; k++) {
    sum2_low += a_low[k] * ybuff_low[k];
  }

  float filter_sum_low = sum1_low - sum2_low;

  for(int k = N-1; k>=1; k--) {
    ybuff_low[k+1] = ybuff_low[k];
  }
  ybuff_low[1] = filter_sum_low;

  //High pass filter
    static float xbuff_high[M+1] = {0};
    static float b_high[M+1] = {
        0.9949860584423,   -2.984958175327,    2.984958175327,  -0.9949860584423
    };
    static float ybuff_high[N+1] = {0};
    static float a_high[N+1] = {
        1,   -2.989946914092,    2.979944296952,  -0.9899972564945
    };
    int invalue_high = norm_value;
    //int invalue_high = filter_sum_low;
    for(int k = M-1; k>=0; k--) {
        xbuff_high[k+1]=xbuff_high[k];
    }
    xbuff_high[0] = (float)invalue_high;

    float sum1_high = 0;
    for(int k = 0; k <= M; k++) {
        sum1_high += b_high[k] * xbuff_high[k];
    }

    float sum2_high = 0;
    for(int k = 1; k <= N; k++) {
        sum2_high += a_high[k] * ybuff_high[k];
    }

    float filter_sum_high = sum1_high - sum2_high;

    for(int k = N-1; k>=1; k--) {
        ybuff_high[k+1] = ybuff_high[k];
    }
    ybuff_high[1] = filter_sum_high;

    float outvalue = filter_sum_high;
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
    addElement(&normalization,adc_value);

    offset = getAverage(&normalization);

    normalized_value = adc_value - offset;
    filtered_value = (int) filter(normalized_value);
    //filtered_value = normalized_value;
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