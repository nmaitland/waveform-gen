#ifndef WAVEFORM_UTILS_H
#define WAVEFORM_UTILS_H

#include <stdint.h>

// Frequency scaling constants
constexpr long MaxAD9833Freq = 12500000L;  // 12.5 MHz
constexpr long MaxStep = 1000000L;

// Frequency label indices
enum FrequencyScale {
    SCALE_HZ = 0,
    SCALE_KHZ = 1,
    SCALE_MHZ = 2
};

// Calculate which frequency scale to use (Hz, KHz, MHz)
inline FrequencyScale getFrequencyScale(long frequency) {
    if (frequency >= 1000000) {
        return SCALE_MHZ;
    } else if (frequency >= 1000) {
        return SCALE_KHZ;
    } else {
        return SCALE_HZ;
    }
}

// Get the divisor for a given frequency scale
inline long getFrequencyDivisor(FrequencyScale scale) {
    switch (scale) {
        case SCALE_MHZ: return 1000000L;
        case SCALE_KHZ: return 1000L;
        case SCALE_HZ:
        default: return 1L;
    }
}

// Clamp frequency to valid range
inline long clampFrequency(long frequency) {
    if (frequency <= 0L) {
        return 1L;
    }
    if (frequency > MaxAD9833Freq) {
        return MaxAD9833Freq;
    }
    return frequency;
}

// Calculate next step value (cycles: 1 -> 10 -> 100 -> ... -> 1M -> 1)
inline long nextStepValue(long currentStep) {
    long newStep = currentStep * 10;
    if (newStep > MaxStep) {
        return 1;
    }
    return newStep;
}

#endif // WAVEFORM_UTILS_H
