// Wokwi Custom Chip - For docs and examples see:
// https://docs.wokwi.com/chips-api/getting-started
//
// SPDX-License-Identifier: MIT
// Copyright 2023 Neil Maitland

#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// AD9833 pin naming
// REF = 25MHz Reference clock from the on-board oscillator is output on this pin.
// VCC =  Vcc power  (2.3 to 5.5V)
// GND = Ground
// DAT = SPI Bus Data pin (MOSI)
// CLK = SPI Bus Clock pin
// FNC =  SPI Bus Load pin (SS) (called FSYNC on the AD9833)
// OUT =  Output of the function generator.  Also available on center pin of BNC location and Tset

/***********************************************************************
						Control Register
------------------------------------------------------------------------
D15,D14(MSB)	
    10 = FREQ1 write, 
    01 = FREQ0 write, 
    11 = PHASE write, 
    00 = control write
D13	If D15,D14 = 00, 
        0 = individual LSB and MSB FREQ write, 
        1 = both LSB and MSB FREQ writes consecutively
	  If D15,D14 = 11
        0 = PHASE0 write
        1 = PHASE1 write
D12	0 = writing LSB independently, 
    1 = writing MSB independently
D11	0 = output FREQ0,
    1 = output FREQ1
D10	0 = output PHASE0,	
    1 = output PHASE1
D9	Reserved. Must be 0.
D8	0 = RESET disabled, 
    1 = RESET enabled
D7	0 = internal clock is enabled, 
    1 = internal clock is disabled
D6	0 = onboard DAC is active for sine and triangle wave output,
 	  1 = put DAC to sleep for square wave output
D5	0 = output depends on D1
	  1 = output is a square wave
D4	Reserved. Must be 0.
D3	0 = square wave of half frequency output
	  1 = square wave output
D2	Reserved. Must be 0.
D1	If D5 = 1, D1 = 0.
	Otherwise 
    0 = sine output, 
    1 = triangle output
D0	Reserved. Must be 0.
***********************************************************************/

//  CONTROL REGISTER BITS
#define AD9833_WR1      (1 << 15)  // 0x8000
#define AD9833_WR0      (1 << 14)  // 0x4000
#define AD9833_B28      (1 << 13)  // 0x2000
#define AD9833_HLB      (1 << 12)  // 0x1000
#define AD9833_FSELECT  (1 << 11)  // 0x0800
#define AD9833_PSELECT  (1 << 10)  // 0x0400
#define AD9833_RESET    (1 << 8)   // 0x0100
#define AD9833_SLEEP1   (1 << 7)   // 0x0080
#define AD9833_SLEEP12  (1 << 6)   // 0x0040
#define AD9833_OPBITEN  (1 << 5)   // 0x0020
#define AD9833_DIV2     (1 << 3)   // 0x0008
#define AD9833_MODE     (1 << 1)   // 0x0002
uint32_t referenceFrequency = 25000000UL;
#define pow2_28				268435456L	// 2^28 used in frequency word calculation

typedef struct {
  pin_t    cs_pin;
  uint32_t spi;
  uint8_t  spi_buffer[128];
  int32_t lower14;
  int32_t upper14;
  char* waveform;
  uint8_t pchannel;
  uint8_t fchannel;
  bool inReset;
} chip_state_t;

static void chip_pin_change(void *user_data, pin_t pin, uint32_t value);
static void chip_spi_done(void *user_data, uint8_t *buffer, uint32_t count);

void chip_init(void) {
  chip_state_t *chip = malloc(sizeof(chip_state_t));
  
  chip->cs_pin = pin_init("FNC", INPUT_PULLUP);
  chip->lower14 = -1;
  chip->upper14 = -1;
  chip->waveform = "";
  chip->pchannel = 99;
  chip->fchannel = 99;
  chip->inReset = false;

  const pin_watch_config_t watch_config = {
    .edge = BOTH,
    .pin_change = chip_pin_change,
    .user_data = chip,
  };
  pin_watch(chip->cs_pin, &watch_config);

  const spi_config_t spi_config = {
    .sck = pin_init("CLK", INPUT),
    .mosi = pin_init("DAT", INPUT),
    .miso = pin_init("MISO", INPUT),
    .done = chip_spi_done,
    .user_data = chip,
  };
  chip->spi = spi_init(&spi_config);
  
  printf("SPI Chip initialized!\n");
}

void chip_pin_change(void *user_data, pin_t pin, uint32_t value) {
  chip_state_t *chip = (chip_state_t*)user_data;

  // Handle CS pin logic
  if (pin == chip->cs_pin) {
    if (value == LOW) {
      // printf("SPI chip deselected\n");
      spi_start(chip->spi, chip->spi_buffer, 0);
    } else {
      // printf("SPI chip selected\n");
      spi_stop(chip->spi);
    }
  }
}

void chip_spi_done(void *user_data, uint8_t *buffer, uint32_t count) {
  chip_state_t *chip = (chip_state_t*)user_data;

  if (!count) {
    // This means that we got here from spi_stop, and no data was received
    printf("AD9833 Received 0 bytes\n");
    return;
  }
  else if (count > 4) {
    printf("Unrecognised command - received %d bytes. Ignoring\n", count);
    return;
  }

  uint16_t command = (buffer[0] << 8) | buffer[1];
  printf("Command is: 0x%04X\n", command);

  if ((command & AD9833_WR1) || (command & AD9833_WR0)) {
    // set frequency or phase
    if ((command & AD9833_WR1) && (command & AD9833_WR0)) {
     //set PHASE
     if (command & AD9833_B28) {
        // set PHASE1
        printf("Set PHASE1: ");
      }
      else {
        // set PHASE0
        printf("Set PHASE0: ");
      }
      float phase = (float)(command & 0x0FFF) * 360.0 / 4095.0;
        printf("%3.2f\n", phase);          
    }
    else {
      uint8_t freqChannel = 0;
      if (command & AD9833_WR1) {
        freqChannel = 1;
      }

      if (command | AD9833_B28) {
        // both LSB and MSB FREQ writes consecutively
        if (count < 4) {
          printf("Unrecognised B28 command - expected 4 bytes, rxed %d bytes. Ignoring\n", count);
          return;
        }
        chip->lower14 = (int16_t)(command & 0x3FFF);
        chip->upper14 = (int16_t)((buffer[2] << 8) | buffer[3]);
      }
      else {
        // individual LSB and MSB FREQ write        
        // printf("individual LSB and MSB FREQ write\n");
        if(command & AD9833_HLB) {
          chip->lower14 = (int16_t)(command & 0x3FFF);
          // printf("Writing LSB %04x\n", chip->lower14);
        }
        else {
          chip->upper14 = (int16_t)(command & 0x3FFF);
          // printf("Writing MSB %04x\n", chip->upper14);
        }
      } 

      if (chip->upper14 >= 0 && chip->lower14 >= 0) {
        int32_t freqWord = chip->lower14;
        freqWord |= (chip->upper14 << 14);
        float frequency = ((((float)freqWord) * referenceFrequency) / pow2_28) - referenceFrequency;
        printf("Setting FREQ%d to: %f\n", freqChannel, frequency);
        chip->lower14 = chip->upper14 = -1;
      }

    }
  }
  else {
    // set command register
    if ((command & AD9833_RESET) && !chip->inReset) {
        printf("State: RESET\n");
        chip->inReset = true;
        return; // this disables output, but doesn't change registers - so stop processing
    }
    else if (!(command & AD9833_RESET) && chip->inReset) {
        printf("State: RUNNING\n");
        chip->inReset = false;
    }

    uint8_t newFChannel = 0;
    if (command & AD9833_FSELECT) {
      newFChannel = 1;
    }
    if (newFChannel != chip->fchannel) {
      printf("Frequency Channel: %d\n", newFChannel);
      chip->fchannel = newFChannel;
    }
    
    uint8_t newPChannel = 0;
    if (command & AD9833_PSELECT) {
      newPChannel = 1;
    }
    if (newPChannel != chip->pchannel) {
      printf("Phase Channel: %d\n", newPChannel);
      chip->pchannel = newPChannel;
    }

    char* newWaveform = chip->waveform;
    if ((command & AD9833_SLEEP1) || (command & AD9833_SLEEP12)) {
      newWaveform = "OFF";
    }
    else if ((command & AD9833_DIV2) && (command & AD9833_OPBITEN) && !(command & AD9833_MODE)) {
      newWaveform = "SQUARE1";
    }
    else if ((command & AD9833_OPBITEN) && !(command & AD9833_DIV2) && !(command & AD9833_MODE)) {
      newWaveform = "SQUARE2";
    }
    else if ((command & AD9833_MODE) && !(command & AD9833_OPBITEN)) {
      newWaveform = "TRIANGLE";
    }
    else if (!(command & AD9833_MODE) && !(command & AD9833_OPBITEN)) {
      newWaveform = "SINE";
    }

    if (strcmp(newWaveform, chip->waveform)) {
      printf("Waveform set to: %s\n", newWaveform);
      chip->waveform = newWaveform;
    }
  }
}
