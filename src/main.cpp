#include <RotaryEncoder.h>
#include <LiquidCrystal_I2C.h>
#include <Bounce2.h>
#include <AD9833.h>
#include <SPI.h>

LiquidCrystal_I2C lcd(0x27,16,2); //  0x3F or 0x27

#define PIN_IN1 2
#define PIN_IN2 3

// SPI: 10 (SS), 11 (MOSI), 12 (MISO), 13 (SCK).


// A pointer to the dynamic created rotary encoder instance.
// This will be done in setup()
RotaryEncoder *encoder = nullptr;
Bounce wfButton = Bounce();
Bounce scaleButton = Bounce();
AD9833 AD;

// setup data for rotary encoder acceleration
// the maximum acceleration is 10 times.
constexpr float rotaryAcceleration = 10;
// at 200ms or slower, there should be no acceleration. (factor 1)
constexpr float longCutoff = 50;
// at 5 ms, we want to have maximum acceleration (factor rotaryAcceleration)
constexpr float shortCutoff = 5;

// To derive the calc. constants, compute as follows:
// On an x(ms) - y(factor) plane resolve a linear formula factor(ms) = rotaryMultiplier * ms + rotaryK;
// where  f(4)=10 and f(200)=1
constexpr float rotaryMultiplier = (rotaryAcceleration - 1) / (shortCutoff - longCutoff);
constexpr float rotaryK = 1 - longCutoff * rotaryMultiplier;

// a global variables to hold the last position
static bool newFrequency = false;
static bool newWaveform = false;
static bool newScale = false;
static long lastRotaryPos = 0;
static float curFrequency = 0.0;
typedef struct {
  uint32_t multiplier;
  const char* name;
} FrequencyScale;
static FrequencyScale frequencyScales[] = {
  { 1, "Hz " },
  { 1000, "KHz" },
  { 1000000, "MHz" }
};
static uint8_t curScale = 0;

typedef struct {
  uint8_t waveform;
  const char* name;
} Waveform;
static Waveform waveforms[] = {
  { AD9833_OFF, "OFF" },
  { AD9833_SINE, "SIN" },
  { AD9833_SQUARE1, "SQU" },
  { AD9833_TRIANGLE, "TRI" }
};
static uint8_t curWaveform = 0;

// This interrupt routine will be called on any change of one of the input signals
void checkPosition()
{
  encoder->tick(); // just call tick() to check the state.
}

void refreshOutputs()
{
  if (newFrequency || newScale || newWaveform) {
    Serial.print("Refreshing output: fx:"); Serial.print(newFrequency?"Y":"N");
    Serial.print(" sc:"); Serial.print(newScale?"Y":"N");
    Serial.print(" wf:"); Serial.print(newWaveform?"Y":"N");
    Serial.println();
  }

  if (newScale) {
    curScale++;
    if (curScale >= sizeof(frequencyScales)/sizeof(FrequencyScale)) {
      curScale = 0;
    }
    curFrequency = frequencyScales[curScale].multiplier;
    encoder->setPosition(1);
    lcd.setCursor(13, 0);
    lcd.print(frequencyScales[curScale].name);
  }
  
  if (newFrequency || newScale) {
    AD.setFrequency(curFrequency);
    lcd.setCursor(4, 0);
    lcd.print(curFrequency/(float)(frequencyScales[curScale].multiplier));
  } 

  if (newWaveform) {
    curWaveform++;
    if (curWaveform >= sizeof(waveforms)/sizeof(Waveform)) {
      curWaveform = 0;
    }
    lcd.setCursor(13, 1);
    lcd.print(waveforms[curWaveform].name);
  }

  newScale = newFrequency = newWaveform = false;
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("Starting");

  wfButton.attach(9, INPUT_PULLUP);
  wfButton.interval(25);
  scaleButton.attach(4, INPUT_PULLUP);
  scaleButton.interval(25);

  lcd.init();   // LCD 
  lcd.backlight();  // LCD
  lcd.setCursor(0, 0);
  lcd.print("Fx: ");
    lcd.setCursor(13, 1);
    lcd.print(waveforms[curWaveform].name);
    lcd.setCursor(13, 0);
    lcd.print(frequencyScales[curScale].name);

  // setup the rotary encoder functionality

  // use FOUR3 mode when PIN_IN1, PIN_IN2 signals are always HIGH in latch position.
  // encoder = new RotaryEncoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::FOUR3);

  // use FOUR0 mode when PIN_IN1, PIN_IN2 signals are always LOW in latch position.
  // encoder = new RotaryEncoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::FOUR0);

  // use TWO03 mode when PIN_IN1, PIN_IN2 signals are both LOW or HIGH in latch position.
  encoder = new RotaryEncoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::FOUR3);

  // register interrupt routine
  attachInterrupt(digitalPinToInterrupt(PIN_IN1), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_IN2), checkPosition, CHANGE);

  // AD.begin(10, 11, 13);  //  HW SPI, select pin 10
  AD.begin(10, &SPI);
  Serial.println("AD9833 started");
  encoder->setPosition(1);
}

void loop() {
  wfButton.update();
  scaleButton.update();
  encoder->tick(); // just call tick() to check the state.

  long newPos = encoder->getPosition();
  if (lastRotaryPos != newPos) {
    // accelerate when there was a previous rotation in the same direction.
    unsigned long ms = encoder->getMillisBetweenRotations();

    if (ms < longCutoff) {
      // do some acceleration using factors a and rotaryK

      // limit to maximum acceleration
      if (ms < shortCutoff) {
        ms = shortCutoff;
      }

      float ticksActual_float = rotaryMultiplier * ms + rotaryK;
      long deltaTicks = (long)ticksActual_float * (newPos - lastRotaryPos);
      newPos = newPos + deltaTicks;
    }

    if (newPos <= 0L) {
      newPos = 1;
    }

    curFrequency = newPos * frequencyScales[curScale].multiplier;
    if (curFrequency > (float)AD9833_MAX_FREQ) {
      curFrequency = (float)AD9833_MAX_FREQ;
      newPos = 13;
    }
    encoder->setPosition(newPos);

    Serial.print("new pos: ");
    Serial.print(newPos);
    Serial.print("  ms: ");
    Serial.println(ms);
    lastRotaryPos = newPos;
    newFrequency = true;
  } // if

  if (wfButton.fell()) {
    Serial.println("Waveform Button pressed");
    newWaveform = true;
  }

  if (scaleButton.fell()) {
    newScale = true;
  }

  refreshOutputs();
}
