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
static bool frequencyChanged = false;
static bool newWaveform = false;
static bool newStep = false;
static long curFrequency = 1;
static const char* frequencyLabels[] = {
  "Hz ", "KHz", "MHz"
};

static long curStep = 1;
constexpr long MaxStep = 1000000;

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
  // if (frequencyChanged || newStep || newWaveform) {
    // Serial.print("Refreshing output: fx:"); Serial.print(frequencyChanged?"Y":"N");
    // Serial.print(" sc:"); Serial.print(newStep?"Y":"N");
    // Serial.print(" wf:"); Serial.print(newWaveform?"Y":"N");
    // Serial.println();
  // }

  if (newStep) {
    curStep *= 10;
    if (curStep > MaxStep) {
      curStep = 1;
    }
    lcd.setCursor(0, 1);
    lcd.print("+       ");
    lcd.setCursor(1, 1);
    lcd.print(curStep);
  }
  
  if (frequencyChanged) {
    AD.setFrequency((float)curFrequency);

    lcd.setCursor(13, 0);
    int index = (sizeof(frequencyLabels)/sizeof(const char*))-1;
    long mult = 1000000;
    for (; index >= 0; index--, mult /= 1000) {
      if (curFrequency/mult >= 1.0) {
        break;
      }
    }
    lcd.print(frequencyLabels[index]);
    lcd.setCursor(3, 0);
    lcd.print("          ");
    lcd.setCursor(3, 0);
    lcd.print((float)curFrequency/ mult, 6);
  } 

  if (newWaveform) {
    curWaveform++;
    if (curWaveform >= sizeof(waveforms)/sizeof(Waveform)) {
      curWaveform = 0;
    }
    lcd.setCursor(13, 1);
    lcd.print(waveforms[curWaveform].name);
  }

  newStep = frequencyChanged = newWaveform = false;
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
  lcd.print(frequencyLabels[0]);

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

  long newFrequency = encoder->getPosition();
  if (curFrequency != newFrequency) {
    Serial.print("Lst Pos: "); Serial.print(curFrequency);
    Serial.print(" New Pos: "); Serial.println(newFrequency);
    // accelerate when there was a previous rotation in the same direction.
    unsigned long ms = encoder->getMillisBetweenRotations();
    Serial.print("ms: "); Serial.println(ms);

    long deltaTicks = 1;
    if (ms < longCutoff) {
      // do some acceleration using factors a and rotaryK

      // limit to maximum acceleration
      if (ms < shortCutoff) {
        ms = shortCutoff;
      }

      float ticksActual_float = rotaryMultiplier * ms + rotaryK;
      long deltaTicks = (long)ticksActual_float * (newFrequency - curFrequency);
      newFrequency = newFrequency + (deltaTicks * curStep);
    }
    else {
      newFrequency = curFrequency + ((newFrequency - curFrequency) * curStep);
    }

    if (newFrequency <= 0L) {
      newFrequency = 1;
    }
    else if (newFrequency > AD9833_MAX_FREQ) {
      newFrequency = AD9833_MAX_FREQ;
    }
    encoder->setPosition(newFrequency);

    frequencyChanged = true;
    Serial.println("---");
    Serial.print("Lst Pos: "); Serial.print(curFrequency);
    Serial.print(" New Pos: "); Serial.println(newFrequency);
    curFrequency = newFrequency;
 } // if

  if (wfButton.fell()) {
    // Serial.println("Waveform Button pressed");
    newWaveform = true;
  }

  if (scaleButton.fell()) {
    newStep = true;
  }

  refreshOutputs();
}
