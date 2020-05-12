/**************************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers

 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98

 This example is for a 128x64 pixel display using I2C to communicate
 3 pins are required to interface (two I2C and one reset).

 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source
 hardware by purchasing products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries,
 with contributions from the open source community.
 BSD license, check license.txt for more information
 All text above, and the splash screen below must be
 included in any redistribution.
 **************************************************************************/
 /**
 * Battery level code inspired by code at: 
 * https://github.com/makeabilitylab/arduino/tree/master/ESP32/BatteryLevel
 *
 *
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

// OLED constants: 
const int SCREEN_WIDTH = 128; // OLED display width, in pixels
const int SCREEN_HEIGHT = 64; // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
const int OLED_RESET = 4; // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Accelerometer constants and globals:

// Software SPI port
const int LIS3DH_CS = 14;
Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS);

int _xAccelVal = 0;
int _yAccelVal = 0;
int _zAccelVal = 0;

// Define the number of samples to keep track of. The higher the number, the
// more the readings will be smoothed, but the slower the output will respond to
// the input. Using a constant rather than a normal variable lets us use this
// value to determine the size of the readings array.
const float SAMPLING_RATE = 35.0;
const int BUFFER_SIZE = (int)SAMPLING_RATE;

int _bufferTotal = 0;
int _minPeakHeight = 1000;

unsigned long _currTime;

unsigned long _minDistanceBetweenPeaksMs = 300;
unsigned long _lastPeakTimestamp = -1;

int _circularBuffer[BUFFER_SIZE]; //fast way to store values (rather than an ArrayList with remove calls)
int _readIndex = 0; // tracks where we are in the circular buffer

unsigned long _bufferTimestamps[BUFFER_SIZE];
int _smoothedSignal[BUFFER_SIZE]; // smoothed signal values after averaging

// Battery constants and globals:
const int BATTERY_PIN = A13;
const int MAX_ANALOG_VAL = 4095;
const float MAX_BATTERY_VOLTAGE = 4.2; // Max LiPoly voltage of a 3.7 battery is 4.2

const int BATTERY_WIDTH = 14;
const int BATTERY_HEIGHT = 20;

int _batteryPercent = 100;

int _displayMode = 1;

int _numSteps = 0;

void setup() {
  Serial.begin(9600);
  _currTime = millis();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1) yield();
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // initialize all the readings to 0:
  for (int i = 0; i < BUFFER_SIZE; i++) {
    _circularBuffer[i] = 0;
    _smoothedSignal[i] = 0;
    _bufferTimestamps[i] = 0;
  }

  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);
  display.display();
  delay(1000);
}

// modes: visualization, battery percent, number of steps, miles?

void loop() {  
  countSteps();
  updateBatteryPercent();
  // TODO figure out mode, update display if relevant info is changed for that mode
  if (_numStepsChanged || _batteryPercentChanged) {
    updateDisplay();
    _numStepChanged = false;
    _batteryPercentChanged = false;
  }
}

// TODO maybe figure out overlaps between buffer vals
// ... it might be easy if we change countSteps to a while loop and run until buffer is full
// I'm not sure we want to do that, it'll stop all other functionality while running then, I wouldn't be able to change modes
// TODO also figure out the countPeaks loop indices
// TODO more comments, cite sources
// TODO touch input, cycle through device modes
// TODO visualizations
void countSteps() {
  processAccelData();
  // if we're at the end of the array...
  if (_readIndex >= BUFFER_SIZE) {
    // ...wrap around to the beginning:
    _readIndex = 0;
    // count peaks for all values in the buffer
    countPeaks();
  }
}

// Read accelerometer values and smooth the signal. Count peaks when buffer is full
void processAccelData() {
  lis.read(); 
  int sensorVal = 0;
  int average = 0;

  long x = lis.x;
  long y = lis.y;
  long z = lis.z;
  double mag = sqrt(x * x + y * y + z * z);     
  sensorVal = mag;
  
  // subtract the last reading:
  _bufferTotal = _bufferTotal - _circularBuffer[_readIndex];
  // read from the sensor:
  _circularBuffer[_readIndex] = sensorVal;
  _bufferTimestamps[_readIndex] = _currTime;
  // add the reading to the total:
  _bufferTotal += sensorVal;

  // calculate the average:
  average = _bufferTotal / BUFFER_SIZE;
  // save average to smoothed signal array after mean substitution
  _smoothedSignal[_readIndex] = sensorVal - average;
  
  // send it to the computer as ASCII digits
  Serial.println(_smoothedSignal[_readIndex]);

  // advance to the next position in the array:
  _readIndex = _readIndex + 1;
  
  delay(1);        // delay in between reads for stability
}

/**
 * Detect peaks in the smoothed signal values if they're above a threshold and far enough apart. 
 * This signal processing approach assumes a fixed placement of the device on the wearer.
 */
void countPeaks() {
  for (int i = 0; i < BUFFER_SIZE-1; i++) {
    int forwardSlope = _smoothedSignal[i + 1] - _smoothedSignal[i];
    int backwardSlope = _smoothedSignal[i] - _smoothedSignal[i - 1];
    if (forwardSlope < 0 && backwardSlope > 0) {
      unsigned long currPeakTimestamp = _bufferTimestamps[i];
      int peakVal = _smoothedSignal[i];
      if (peakVal >= _minPeakHeight) {
        unsigned long timeSinceLastDetectionMs = currPeakTimestamp - _lastPeakTimestamp;
        if(_lastPeakTimestamp == -1 || timeSinceLastDetectionMs >= _minDistanceBetweenPeaksMs){
            _lastPeakTimestamp = currPeakTimestamp;
            _numSteps += 1;
            _numStepsChanged = true;
        }
      }
    }
  }
}

void updateBatteryPercent() {
  int rawValue = analogRead(BATTERY_PIN);

  // Reference voltage on ESP32 is 1.1V
  // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html#adc-calibration
  float voltageLevel = (rawValue / 4095.0) * 2 * 1.1 * 3.3; // calculate voltage level
  float batteryFraction = voltageLevel / MAX_BATTERY_VOLTAGE;

  int percent = (int)(batteryFraction*100.0);
  if (percent != _batteryPercent) {
    _batteryPercent = percent;
    _batteryPercentChanged = true;
  }
}

void updateDisplay() {
  display.clearDisplay();
  switch (_displayMode) {
    case 0:
      drawTree();
      break;
    case 1:
      drawStepCount();
      break;
    case 2: 
      drawBattery();
    default:
      break;
  }
  display.display();
}

void drawTree(int percentToRender) {
  
}

void drawStepCount() {
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(50, 32);     // Center three lines of text

  display.print("Step count: ");
  display.println(_numSteps);
}

void drawBattery() {
  int batteryY = _batteryPercent/5;
  display.drawRoundRect(4, 4, BATTERY_WIDTH, BATTERY_HEIGHT+2, 4, SSD1306_WHITE);
  display.fillRoundRect(4, 4+BATTERY_HEIGHT-batteryY, BATTERY_WIDTH, batteryY+1, 4, SSD1306_WHITE);
  
  display.setCursor(20, 5);
  display.print(_batteryPercent);
  display.println("%");
}
