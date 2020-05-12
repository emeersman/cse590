 /**
 * Battery level code inspired by code at: 
 * https://github.com/makeabilitylab/arduino/tree/master/ESP32/BatteryLevel
 * 
 * OLED graphics code inspired by: 
 * https://learn.adafruit.com/monochrome-oled-breakouts/arduino-library-and-examples
 *
 * Signal smoothing code sourced from:
 * https://www.arduino.cc/en/tutorial/smoothing
 * 
 * Peak detection code sourced from:
 * https://colab.research.google.com/github/makeabilitylab/signals/blob/master/Projects/StepTracker/StepTracker-Exercises.ipynb
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

// OLED

// Display constants: 
const int DISPLAY_WIDTH = 128; // OLED display width, in pixels
const int DISPLAY_HEIGHT = 64; // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
const int OLED_RESET = 4; // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, OLED_RESET);

// Tree levels constrained by display width, must ensure top row of branches
// takes up (at most) every other pixel.
const int NUM_TREE_LEVELS = 6;

// Whether to update the display with new information
boolean _displayChanged = false;


// ACCELEROMETER

// Software SPI port
const int LIS3DH_CS = 14;
Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS);

int _xAccelVal = 0;
int _yAccelVal = 0;
int _zAccelVal = 0;


// BATTERY

const int BATTERY_PIN = A13;
const int MAX_ANALOG_VAL = 4095;
const float MAX_BATTERY_VOLTAGE = 4.2; // Max LiPoly voltage of a 3.7 battery is 4.2

float _batteryPercent = 1.0;


// CAPACATIVE TOUCH SENSING

int _displayMode = 0;
unsigned long _lastTouchTimestamp = 0;

const int FORWARD_TOUCH_PIN = T5;
const int BACKWARD_TOUCH_PIN = T7;

// Time gap between reads from the touch sensors.
const unsigned long TOUCH_TIME_INTERVAL = 1000;
const int MIN_TOUCH_THRESHOLD = 20;
const int MAX_TOUCH_THRESHOLD = 30;


// SIGNAL PROCESSING

// The number of samples to keep track of. The higher the number, the
// more the readings will be smoothed, but the slower the output will respond to
// the input.
const float SAMPLING_RATE = 35.0;
const unsigned long MIN_DISTANCE_BETWEEN_PEAKS_MS = 300;
const int MIN_PEAK_HEIGHT = 3000;

const int BUFFER_SIZE = (int)SAMPLING_RATE;

typedef struct {
    unsigned long timestamp;
    int accelVal;
} AccelReading;

AccelReading _smoothedCircularBuffer[BUFFER_SIZE] = {}; // smoothed values and timestamps
int _circularBuffer[BUFFER_SIZE]; // raw accel magnitude values

int _writeIndex = 0; // tracks where we are in the circular buffer
int _bufferTotal = 0; // sum of values in the buffer
int _prevLastSignal = 0;
unsigned long _lastPeakTimestamp = -1;


// STEP COUNTING

const int STEP_GOAL = 50;
int _numSteps = 0;

/** Setup function */
void setup() {
  Serial.begin(9600);
  pinMode(FORWARD_TOUCH_PIN, INPUT);
  pinMode(BACKWARD_TOUCH_PIN, INPUT);

  // OLED setup
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Accelerometer setup
  if (! lis.begin(0x18)) {
    Serial.println("Couldnt start");
    while (1) yield();
  }

  // initialize all the raw values to 0
  for (int i = 0; i < BUFFER_SIZE; i++) {
    _circularBuffer[i] = 0;
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000);
  display.clearDisplay();
}

/** Loops through touch detection, step counting, and display updates. */
void loop() {
  unsigned long timeSinceLastTouchMs = millis() - _lastTouchTimestamp;
  // Only check for new touches if it has been long enough since the last one.
  if (timeSinceLastTouchMs > TOUCH_TIME_INTERVAL) {
    // Detect touch and cycle through display modes.
    int forwardTouch = touchRead(FORWARD_TOUCH_PIN);
    int backwardTouch = touchRead(BACKWARD_TOUCH_PIN);

    if (forwardTouch < MAX_TOUCH_THRESHOLD && forwardTouch > MIN_TOUCH_THRESHOLD) {
      _displayMode++;
      if (_displayMode > 2) {
        _displayMode = 0;
      }
      _lastTouchTimestamp = millis();
      _displayChanged = true;
    } else if (backwardTouch < MAX_TOUCH_THRESHOLD && backwardTouch > MIN_TOUCH_THRESHOLD) {
      _displayMode--;
      if (_displayMode < 0) {
        _displayMode = 2;
      }
      _lastTouchTimestamp = millis();
      _displayChanged = true;
    }
  }
  
  countSteps();
  updateBatteryPercent();
  // Only update display when something changes.
  if (_displayChanged) {
    updateDisplay();
    _displayChanged = false;
  }
}

/** 
 * Processes accelerometer signal until the buffer is full, then 
 * resets the read index and triggers peak counting. 
 */
void countSteps() {
  processAccelData();
  // Wrap to beginning of buffer when we reach the end
  if (_writeIndex >= BUFFER_SIZE) {
    _writeIndex = 0;
    // Count peaks for all values in the buffer
    countPeaks();
    // Save last signal in the buffer for next peak counting loop
    _prevLastSignal = _smoothedCircularBuffer[BUFFER_SIZE-1].accelVal;
  }
}

/** Reads accelerometer values and smoothes the signal. */
void processAccelData() {
  int sensorVal = 0;
  int average = 0;

  // Record magnitude of accelerometer values
  lis.read(); 
  long x = lis.x;
  long y = lis.y;
  long z = lis.z;
  double magnitude = sqrt(x * x + y * y + z * z);     
  sensorVal = magnitude;
  
  // Subtract the last reading from this buffer location
  _bufferTotal = _bufferTotal - _circularBuffer[_writeIndex];
  // Insert raw data into buffer
  _circularBuffer[_writeIndex] = sensorVal;
  _bufferTotal += sensorVal;
  // Calculate average
  average = _bufferTotal / BUFFER_SIZE;
  
  // Save average to smoothed signal array after mean substitution
  _smoothedCircularBuffer[_writeIndex] = {millis(), sensorVal - average};
  
  _writeIndex += 1;
  
  delay(1); // delay in between reads for stability
}

/**
 * Detects peaks in the smoothed signal values if they're above a threshold and far enough apart. 
 * This signal processing approach assumes a fixed placement of the device on the wearer.
 */
void countPeaks() {
  // Process all values in the smoothed signal to find peaks
  for (int i = 0; i < BUFFER_SIZE-1; i++) {
    // Prevent index out of bounds error by passing in last signal from previous buffer
    int lastSignal = i == 0 ? _prevLastSignal : _smoothedCircularBuffer[i-1].accelVal;
    int backwardSlope = _smoothedCircularBuffer[i].accelVal - lastSignal;
    int forwardSlope = _smoothedCircularBuffer[i+1].accelVal - _smoothedCircularBuffer[i].accelVal;

    // Peak has been detected
    if (forwardSlope < 0 && backwardSlope > 0) {
      unsigned long currPeakTimestamp = _smoothedCircularBuffer[i].timestamp;
      int peakVal = _smoothedCircularBuffer[i].accelVal;
      
      // Filter out peaks under the minimum height
      if (peakVal >= MIN_PEAK_HEIGHT) {
        // Only detect peaks detected more than a certain amount of time apart
        unsigned long timeSinceLastDetectionMs = currPeakTimestamp - _lastPeakTimestamp;
        if(_lastPeakTimestamp == -1 || timeSinceLastDetectionMs >= MIN_DISTANCE_BETWEEN_PEAKS_MS){
            _lastPeakTimestamp = currPeakTimestamp;
            _numSteps += 1;
            _displayChanged = true;
        }
      }
    }
  }
}

/** Reads battery value and updates global variable if the percentage changes. */
void updateBatteryPercent() {
  int rawValue = analogRead(BATTERY_PIN);
  float voltageLevel = (rawValue / 4095.0) * 2 * 1.1 * 3.3; // calculate voltage level
  float batteryFraction = voltageLevel / MAX_BATTERY_VOLTAGE;

  if (batteryFraction != _batteryPercent) {
    _batteryPercent = batteryFraction;
    // Display needs to be updated
    _displayChanged = true;
  }
}

/** Updates OLED display based on the current display mode. */
void updateDisplay() {
  display.clearDisplay();
  switch (_displayMode) {
    case 0:
      drawTree((float)_numSteps/STEP_GOAL);
      break;
    case 1:
      drawStepCount();
      break;
    case 2: 
      drawBattery();
    default:
      break;
  }
}

/**
 * Draws a binary tree for which the number of levels correspond
 * to a given percentage parameter.
 */
void drawTree(float percentToRender) {
  int numLevels = NUM_TREE_LEVELS;

  // If over 100% reached, display a message. If 100% reached, just display 
  // the full tree (otherwise the message will also appear on the battery tree)
  if (percentToRender > 1.0) {
    display.setCursor(10, 50);
    display.print("GOAL");
    display.setCursor(76, 50);
    display.println("REACHED");
  } else {
    // Set numLevels based on percentage parameter
    float fraction = percentToRender * 8.0;
    numLevels = (int)fraction - 2;
  }
  
  // Draw trunk
  display.fillRect(DISPLAY_WIDTH/2-4, 3*DISPLAY_HEIGHT/4, 8, DISPLAY_HEIGHT/4, SSD1306_WHITE);
  display.drawPixel(DISPLAY_WIDTH/2-5, DISPLAY_HEIGHT-1, SSD1306_WHITE);
  display.drawPixel(DISPLAY_WIDTH/2+4, DISPLAY_HEIGHT-1, SSD1306_WHITE);

  // Draw tree from bottom up
  for (int i = 1; i <= numLevels; i++) {
    for (int j = 0; j < pow(2, i); j++) {
      int add1IfEven = j%2 == 0 ? 1 : 0; // used to calculate x1
      int x1 = DISPLAY_WIDTH/(pow(2,i))*(j+add1IfEven);
      int x2 = DISPLAY_WIDTH/(pow(2,i+1))*(2*j+1);
      int y1 = (NUM_TREE_LEVELS-i+1)*DISPLAY_HEIGHT/8;
      int y2 = (NUM_TREE_LEVELS-i)*DISPLAY_HEIGHT/8;
      display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
    }
  }
  display.display();
}

/** Writes the current step count and step goal. */
void drawStepCount() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(24, 16);
  display.print("Step count: ");
  display.println(_numSteps);

  display.setCursor(24, 40);
  display.print("Step goal: ");
  display.println(STEP_GOAL);
  
  display.display();
}

/* Draws an inverse tree to represent remaining battery percentage. */
void drawBattery() {
  // Write battery percent number across the top of the display
  display.setCursor(8, 5);
  display.print("BATTERY");
  display.setCursor(100, 5);
  int batteryNum = (int)(_batteryPercent*100.0);
  display.print(batteryNum);
  display.println("%");

  // Rotate 180 degrees
  display.setRotation(2);
  drawTree(_batteryPercent);
  
  display.display();

  // Reset display back to 0 degrees
  display.setRotation(0);
}
