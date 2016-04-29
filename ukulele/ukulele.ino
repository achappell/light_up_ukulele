// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            12
#define ONOFFLEDPIN    10
#define ONOFFBUTTONPIN 11
#define COLORLEDPIN     9
#define COLORBUTTONPIN  4

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      60

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 5; // delay for half a second
int currentOnOffState = HIGH;
int currentMode = 0;
int previousColorButtonState = HIGH;
uint32_t rainbowJ = 0;
int isRainbowing = 0;

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  pinMode(ONOFFLEDPIN, OUTPUT);
  pinMode(ONOFFBUTTONPIN, INPUT);
  pinMode(COLORLEDPIN, OUTPUT);
  pinMode(COLORBUTTONPIN, INPUT);

  pixels.begin(); // This initializes the NeoPixel library.
}

void loop() {

  int onOffButtonState;

  onOffButtonState = digitalRead(ONOFFBUTTONPIN);
  if (onOffButtonState != currentOnOffState) {
    currentOnOffState = onOffButtonState;
    if (onOffButtonState == HIGH) {
      digitalWrite(ONOFFLEDPIN, LOW);
      digitalWrite(COLORLEDPIN, LOW);
      isRainbowing = 0;
    
      for(int i=NUMPIXELS-1;i>=0;i--){
    
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
    
        pixels.show(); // This sends the updated pixel color to the hardware.
        delay(delayval*2);
      } 
    } else {
      digitalWrite(ONOFFLEDPIN, HIGH);
      digitalWrite(COLORLEDPIN, HIGH);
    
      for(int i=0;i<NUMPIXELS;i++){
    
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        pixels.setPixelColor(i, pixels.Color(0,170,50)); // Moderately bright green color.
    
        pixels.show(); // This sends the updated pixel color to the hardware.
        delay(delayval*2);
      }   
    }
  }

  int colorButtonState = digitalRead(COLORBUTTONPIN);

  if (colorButtonState == LOW && previousColorButtonState != colorButtonState) {
    colorForCurrentState();
    currentMode++;
    currentMode = currentMode%6;
  }
  previousColorButtonState = colorButtonState;

  if (isRainbowing == 1) {
    rainbowCycle();
  }

  delay(delayval);
}

void colorForCurrentState() {
  if (currentMode >= 0 && currentMode < 5) {
    isRainbowing = 0;
    uint32_t color = Wheel(currentMode*50);
    
    for(int i=0;i<NUMPIXELS;i++){
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        pixels.setPixelColor(i, color); // Moderately bright green color
    }
    pixels.show();
  } else {
    isRainbowing = 1;
    rainbowCycle();
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle() {
  uint16_t i, j;
  for(i=0; i< pixels.numPixels(); i++) {
    pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + rainbowJ) & 255));
  }
  pixels.show();
  rainbowJ++;
}

void rainbow() {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel((i+j) & 255));
    }
    pixels.show();
    delay(5);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

