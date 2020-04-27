#include "src/RGBConverter/RGBConverter.h"

const int BUTTON_PIN = 2;
const int BLUE_RGB_PIN = 3;
const int GREEN_RGB_PIN = 5;
const int RED_RGB_PIN = 6;
const int FROG_PIN = 7;
const int PHOTOCELL_PIN = A0;
const int MICROPHONE_PIN = A1;
const int LO_FI_INPUT_PIN = A2;

// Current night light mode
int _buttonMode = 0;


// Mode 1 globals and constants: 

// Photoresistor constants and logic informed by the lesson at:
// https://makeabilitylab.github.io/physcomp/sensors/photoresistors.html

// Light level threshold to begin turning on LED (LED should be off until 200 threshold reached)
const int MIN_PHOTOCELL_RANGE = 200; 
// Max darkness level (LED should be fully on for anything 800+)
const int MAX_PHOTOCELL_RANGE = 800; 

// Crossfade logic informed by the lesson at:
// https://makeabilitylab.github.io/physcomp/arduino/rgb-led-fade.html
float _hue = 0;
float _step = 0.005f;
RGBConverter _rgbConverter;


// Mode 2 globals and constants:

// Sound processing code from:
// https://learn.adafruit.com/adafruit-microphone-amplifier-breakout/measuring-sound-levels

const int SAMPLE_WINDOW = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int _sample;


// Mode 3 globals and constants:

const int LOFI_MIN = 0;
const int LOFI_MAX = 500;

int _currentInputVal = 0;


// Frog mode globals and constants:

// Blink without delay code inspired by the lesson at:
// https://makeabilitylab.github.io/physcomp/arduino/led-blink.html

const int INTERVAL_IN_MS = 200; // interval at which to blink (in milliseconds)

boolean _isFrogMode = false;
unsigned long _lastToggledTimestampMs = 0; // tracks the last time the LED was updated


// Setup function
void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(PHOTOCELL_PIN, INPUT);
  pinMode(MICROPHONE_PIN, INPUT);
  pinMode(LO_FI_INPUT_PIN, INPUT);
  pinMode(FROG_PIN, INPUT_PULLUP);
  pinMode(BLUE_RGB_PIN, OUTPUT);
  pinMode(GREEN_RGB_PIN, OUTPUT);
  pinMode(RED_RGB_PIN, OUTPUT);
}

// Sets the color of the RGB LED, factored by an optional brightness value
void setColor(int red, int green, int blue, float brightness = 1.0) {
  analogWrite(RED_RGB_PIN, red*brightness);
  analogWrite(GREEN_RGB_PIN, green*brightness);
  analogWrite(BLUE_RGB_PIN, blue*brightness);
}

// Loop function
void loop() {
  // Cycle through four possible modes whenever mode button is pressed
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (_buttonMode == 3) {
      _buttonMode = 0;
    } else {
      _buttonMode++;
    }
    // Blink RGB LED to signal new mode
    setColor(255, 255, 255);
    delay(100);
    setColor(0, 0, 0);
    delay(500); // delay after button is pressed
  }

  // Toggle between frog mode whenever frog button is presssed
  if (digitalRead(FROG_PIN) == LOW) {
    _isFrogMode = !_isFrogMode;
    delay(200); // delay after button is pressed
  }

  if (_isFrogMode) { // frog mode overrides all other modes
    frogMode();   
  } else if (_buttonMode == 1) {
    setInverseBrightness();
  } else if (_buttonMode == 2) {
    handleSound();
  } else if (_buttonMode == 3) {
    handleLoFiInput();
  }

  delay(20);
}

// Read ambient light values and inversely adjust RGB LED brightness.
void setInverseBrightness() {
  int photocellVal = analogRead(PHOTOCELL_PIN);

  // Map photocell value to brightness float between 0 and 1
  int ledBrightness = map(photocellVal, MIN_PHOTOCELL_RANGE, MAX_PHOTOCELL_RANGE, 0, 1000);
  ledBrightness = constrain(ledBrightness, 0, 1000);  
  float fLedBrightness = (float)ledBrightness/1000.0;
  
  // Convert current hue, saturation, and lightness to RGB
  byte rgb[3];
  _rgbConverter.hslToRgb(_hue, 1, 0.5, rgb);

  setColor(rgb[0], rgb[1], rgb[2], fLedBrightness); 

  // update hue based on step size
  _hue += _step;

  // hue ranges between 0-1, so if > 1, reset to 0
  if(_hue > 1.0){
    _hue = 0;
  }
  delay(100);
}

// Read sound levels from microphone and adjust RGB LED brightness accordingly.
void handleSound() {
  unsigned long startMillis= millis();  // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level
 
  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;
 
  // collect data for 50 mS
  while (millis() - startMillis < SAMPLE_WINDOW) {
    _sample = analogRead(MICROPHONE_PIN);
    if (_sample < 1024) {  // toss out spurious readings
      if (_sample > signalMax) {
        signalMax = _sample;  // save just the max levels
      } else if (_sample < signalMin) {
        signalMin = _sample;  // save just the min levels
      }
    }
  }
  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude

  double volts = (peakToPeak * 3.3) / 1024;  // convert to volts
  volts = constrain(volts, 0, 1.0); // Constrain based on input range

  int ledVal = (int)(volts*255.0); // Convert to rgb values
  setColor(ledVal, 0, ledVal);
  delay(50);
}

// Read lo-fi input and convert it to a hue in order to change the RGB LED color.
void handleLoFiInput() {
  int loFiVal = analogRead(LO_FI_INPUT_PIN);
  
  if (loFiVal > 10) { // filter out values when circuit is not complete
    loFiVal = constrain(loFiVal, LOFI_MIN, LOFI_MAX);
    _currentInputVal = map(loFiVal, LOFI_MIN, LOFI_MAX, 0, 1000);
  }
  float fHue = (float)_currentInputVal/1000.0; // get precise value from 0-1 for hue

  // Set RGB LED color based on hue from lo-fi input
  byte mode3rgb[3];
  _rgbConverter.hslToRgb(fHue, 1, 0.4, mode3rgb);

  setColor(mode3rgb[0], mode3rgb[1], mode3rgb[2]); 
  delay(50);
}

// Blink green light on and off when in frog mode
void frogMode() {
  // Get the current time since the Arduino started our program (in ms)
  unsigned long currentTimestampMs = millis();
  int greenVal = 0; // toggles between 0 and 255
  
  // Check to see if time since the LED was last toggled exceeds the interval
  if (currentTimestampMs - _lastToggledTimestampMs >= INTERVAL_IN_MS) {
    greenVal = greenVal == 0 ? 255 : 0; // toggle the LED
    _lastToggledTimestampMs = currentTimestampMs; // store current time as toggle time
    setColor(0, greenVal, 0);
  }
}
