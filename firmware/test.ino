#include <Adafruit_NeoPixel.h>

// =======================
// MATRIX CONFIG
// =======================
#define NUM_ROWS 5
#define NUM_COLS 5

// Column pins (outputs)
uint8_t colPins[NUM_COLS] = {2, 3, 4, 5, 6};

// Row pins (inputs)
uint8_t rowPins[NUM_ROWS] = {11, 10, 9, 8, 7};

// Thumb row only uses last 2 columns
bool validKey[NUM_ROWS][NUM_COLS] = {
  {1,1,1,1,1},
  {1,1,1,1,1},
  {1,1,1,1,1},
  {1,1,1,1,1},
  {0,0,0,1,1} // row 4 (thumb row)
};

// =======================
// LED CONFIG
// =======================
#define LED_PIN PIN_NFC1
#define NUM_LEDS 22

Adafruit_NeoPixel leds(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// =======================
// STATE TRACKING
// =======================
bool keyState[NUM_ROWS][NUM_COLS];
bool lastKeyState[NUM_ROWS][NUM_COLS];

unsigned long lastDebounceTime[NUM_ROWS][NUM_COLS];
#define DEBOUNCE_DELAY 10 // ms

// =======================
// LED INDEX MAPPING
// =======================
// Maps matrix position → LED index
int keyToLEDIndex[NUM_ROWS][NUM_COLS];
int ledCounter = 0;

// =======================
// SETUP
// =======================
void setup() {
  Serial.begin(115200);
  delay(1000);

  // Setup columns (outputs)
  for (int c = 0; c < NUM_COLS; c++) {
    pinMode(colPins[c], OUTPUT);
    digitalWrite(colPins[c], HIGH);
  }

  // Setup rows (inputs with pullups)
  for (int r = 0; r < NUM_ROWS; r++) {
    pinMode(rowPins[r], INPUT_PULLUP);
  }

  // Build LED mapping
  for (int r = 0; r < NUM_ROWS; r++) {
    for (int c = 0; c < NUM_COLS; c++) {
      if (validKey[r][c]) {
        keyToLEDIndex[r][c] = ledCounter++;
      } else {
        keyToLEDIndex[r][c] = -1;
      }
    }
  }

  leds.begin();
  leds.clear();
  leds.show();

  startupAnimation();
}

// =======================
// STARTUP ANIMATION
// =======================
void startupAnimation() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds.clear();
    leds.setPixelColor(i, leds.Color(0, 50, 0)); // green
    leds.show();
    delay(50);
  }

  leds.clear();
  leds.show();
}

// =======================
// MAIN LOOP
// =======================
void loop() {
  scanMatrix();
}

// =======================
// MATRIX SCAN
// =======================
void scanMatrix() {
  for (int c = 0; c < NUM_COLS; c++) {

    // Activate column (LOW)
    digitalWrite(colPins[c], LOW);

    for (int r = 0; r < NUM_ROWS; r++) {

      if (!validKey[r][c]) continue;

      bool reading = (digitalRead(rowPins[r]) == LOW);

      if (reading != lastKeyState[r][c]) {
        lastDebounceTime[r][c] = millis();
      }

      if ((millis() - lastDebounceTime[r][c]) > DEBOUNCE_DELAY) {

        if (reading != keyState[r][c]) {
          keyState[r][c] = reading;

          if (reading) {
            onKeyPress(r, c);
          } else {
            onKeyRelease(r, c);
          }
        }
      }

      lastKeyState[r][c] = reading;
    }

    // Deactivate column
    digitalWrite(colPins[c], HIGH);
  }
}

// =======================
// EVENT HANDLERS
// =======================
void onKeyPress(int r, int c) {
  Serial.print("PRESS  R");
  Serial.print(r);
  Serial.print(" C");
  Serial.println(c);

  int ledIndex = keyToLEDIndex[r][c];
  if (ledIndex >= 0) {
    leds.setPixelColor(ledIndex, leds.Color(50, 50, 50)); // white
    leds.show();
  }
}

void onKeyRelease(int r, int c) {
  Serial.print("RELEASE R");
  Serial.print(r);
  Serial.print(" C");
  Serial.println(c);

  int ledIndex = keyToLEDIndex[r][c];
  if (ledIndex >= 0) {
    leds.setPixelColor(ledIndex, 0);
    leds.show();
  }
}