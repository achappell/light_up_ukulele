// Audio Tone Input
// Copyright 2013 Tony DiCola (tony@tonydicola.com)

// This code is part of the guide at http://learn.adafruit.com/fft-fun-with-fourier-transforms/

#define ARM_MATH_CM4
#include <arm_math.h>
#include <Adafruit_NeoPixel.h>


////////////////////////////////////////////////////////////////////////////////
// CONIFIGURATION 
// These values can be changed to alter the behavior of the spectrum display.
////////////////////////////////////////////////////////////////////////////////

int detectedNoteIndex = -1;            // The index of the note we currently here
int SAMPLE_RATE_HZ = 1200;             // Sample rate of the audio in hertz.
const int NOTE[] = {
  392 , 262, 330, 440
};
int idunno = 0;
const int NOTE_COUNT = 4;
int TONE_ERROR_MARGIN_HZ = 20;         // Allowed fudge factor above and below the bounds for each tone input.
float TONE_THRESHOLD_DB = 10.0;        // Threshold (in decibels) each tone must be above other frequencies to count.
const int FFT_SIZE = 256;              // Size of the FFT.  Realistically can only be at most 256 
                                       // without running out of memory for buffers and other state.
const int AUDIO_INPUT_PIN = 14;        // Input ADC pin for audio data.
const int ANALOG_READ_RESOLUTION = 10; // Bits of resolution for the ADC.
const int ANALOG_READ_AVERAGING = 16;  // Number of samples to average with each ADC reading.
const int POWER_LED_PIN = 13;          // Output pin for power LED (pin 13 to use Teensy 3.0's onboard LED).
const int NEO_PIXEL_PIN = 3;           // Output pin for neo pixels.
const int NEO_PIXEL_COUNT = 10;         // Number of neo pixels.  You should be able to increase this without
                                       // any other changes to the program.
const int GAIN_PIN = 16;
const int OUT_PIN = 20;
////////////////////////////////////////////////////////////////////////////////
// INTERNAL STATE
// These shouldn't be modified unless you know what you're doing.
////////////////////////////////////////////////////////////////////////////////

IntervalTimer samplingTimer;
float samples[FFT_SIZE*2];
float magnitudes[FFT_SIZE];
int sampleCounter = 0;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NEO_PIXEL_COUNT, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);


////////////////////////////////////////////////////////////////////////////////
// MAIN SKETCH FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

void setup() {
  // Set up ADC and audio input.
  pinMode(AUDIO_INPUT_PIN, INPUT);
  analogReadResolution(ANALOG_READ_RESOLUTION);
  analogReadAveraging(ANALOG_READ_AVERAGING);
  
  // Turn on the power indicator LED.
  pinMode(POWER_LED_PIN, OUTPUT);
  digitalWrite(POWER_LED_PIN, HIGH);
  
  // Set the gain
  pinMode(GAIN_PIN, OUTPUT);
  digitalWrite(GAIN_PIN, HIGH);
  
  // CHRIS WAS HERE
  // pinMode(OUT_PIN, OUTPUT);
  
  // Initialize neo pixel library and turn off the LEDs
  pixels.begin();
  pixels.show(); 
  
  // Begin sampling audio
  samplingBegin();
}

void loop() {
  // Calculate FFT if a full sample is available.
  if (samplingIsDone()) {
    // Run FFT on sample data.
    arm_cfft_radix4_instance_f32 fft_inst;
    arm_cfft_radix4_init_f32(&fft_inst, FFT_SIZE, 0, 1);
    arm_cfft_radix4_f32(&fft_inst, samples);
    // Calculate magnitude of complex numbers output by the FFT.
    arm_cmplx_mag_f32(samples, magnitudes, FFT_SIZE);

    // Detect tone sequence.
    toneLoop();
  
    // Restart audio sampling.
    samplingBegin();
  }
   
}


////////////////////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

// Compute the average magnitude of a target frequency window vs. all other frequencies.
void windowMean(float* magnitudes, int lowBin, int highBin, float* windowMean, float* otherMean) {
    *windowMean = 0;
    *otherMean = 0;
    // Notice the first magnitude bin is skipped because it represents the
    // average power of the signal.
    for (int i = 1; i < FFT_SIZE/2; ++i) {
      if (i >= lowBin && i <= highBin) {
        *windowMean += magnitudes[i];
      }
      else {
        *otherMean += magnitudes[i];
      }
    }
    *windowMean /= (highBin - lowBin) + 1;
    *otherMean /= (FFT_SIZE / 2 - (highBin - lowBin));
}

// Convert a frequency to the appropriate FFT bin it will fall within.
int frequencyToBin(float frequency) {
  float binFrequency = float(SAMPLE_RATE_HZ) / float(FFT_SIZE);
  return int(frequency / binFrequency);
}

// Convert intensity to decibels
float intensityDb(float intensity) {
  return 20.0*log10(intensity);
}


////////////////////////////////////////////////////////////////////////////////
// SPECTRUM DISPLAY FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

void toneLoop() {
    int pause = 100;

  // find the largest magnitude
  float largestMagnitude = -1000;
  int largestMagnitudeIndex = -1;
  for (int i = 50; i < 192; ++i) {
    if (magnitudes[i] > largestMagnitude) {
      largestMagnitude = magnitudes[i];
      largestMagnitudeIndex = i;
    }
  }
  //TODONE: instead of setting a color here, send this to the trinket
  idunno=largestMagnitudeIndex==-1?0:(largestMagnitudeIndex-49)*256/144;
  //analogWrite(OUT_PIN, 0);
  
  for (int j = 0; j < NEO_PIXEL_COUNT; ++j) {
    pixels.setPixelColor(j, Wheel(idunno));
  }
  pixels.show();
  
}

void toneDetected() {
  int pause = 100;
  int r = 0;
  int g = 0;
  int b = 0;
  switch (detectedNoteIndex) {
    case 0:
      r = 255;
      g = 0;
      b = 0;
      break;
    case 1:
      r = 0;
      g = 255;
      b = 0;
      break;
    case 2:
      r = 0;
      g = 0;
      b = 255;
      break;
    case 3:
      r = 255;
      g = 0;
      b = 255;
      break;
    default:
      r = 0;
      g = 0;
      b = 0;
  }
  // blink the color
  for (int j = 0; j < NEO_PIXEL_COUNT; ++j) {
    pixels.setPixelColor(j, pixels.Color(r, g, b));
  }
  pixels.show();
  delay(pause);
  for (int j = 0; j < NEO_PIXEL_COUNT; ++j) {
    pixels.setPixelColor(j, 0);
  }
  pixels.show();
  delay(pause);
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

////////////////////////////////////////////////////////////////////////////////
// SAMPLING FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

void samplingCallback() {
  // Read from the ADC and store the sample data
  samples[sampleCounter] = (float32_t)analogRead(AUDIO_INPUT_PIN);
  // Complex FFT functions require a coefficient for the imaginary part of the input.
  // Since we only have real data, set this coefficient to zero.
  samples[sampleCounter+1] = 0.0;
  // Update sample buffer position and stop after the buffer is filled
  sampleCounter += 2;
  if (sampleCounter >= FFT_SIZE*2) {
    samplingTimer.end();
  }
}

void samplingBegin() {
  // Reset sample buffer position and start callback at necessary rate.
  sampleCounter = 0;
  samplingTimer.begin(samplingCallback, 1000000/SAMPLE_RATE_HZ);
}

boolean samplingIsDone() {
  return sampleCounter >= FFT_SIZE*2;
}

