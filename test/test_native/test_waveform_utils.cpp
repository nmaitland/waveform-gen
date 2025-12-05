#include <unity.h>
#include "waveform_utils.h"

void setUp(void) {
    // Set up before each test
}

void tearDown(void) {
    // Clean up after each test
}

// Test frequency scale detection
void test_frequency_scale_hz(void) {
    TEST_ASSERT_EQUAL(SCALE_HZ, getFrequencyScale(1));
    TEST_ASSERT_EQUAL(SCALE_HZ, getFrequencyScale(999));
}

void test_frequency_scale_khz(void) {
    TEST_ASSERT_EQUAL(SCALE_KHZ, getFrequencyScale(1000));
    TEST_ASSERT_EQUAL(SCALE_KHZ, getFrequencyScale(999999));
}

void test_frequency_scale_mhz(void) {
    TEST_ASSERT_EQUAL(SCALE_MHZ, getFrequencyScale(1000000));
    TEST_ASSERT_EQUAL(SCALE_MHZ, getFrequencyScale(12500000));
}

// Test frequency divisor calculation
void test_frequency_divisor(void) {
    TEST_ASSERT_EQUAL(1L, getFrequencyDivisor(SCALE_HZ));
    TEST_ASSERT_EQUAL(1000L, getFrequencyDivisor(SCALE_KHZ));
    TEST_ASSERT_EQUAL(1000000L, getFrequencyDivisor(SCALE_MHZ));
}

// Test frequency clamping
void test_clamp_frequency_negative(void) {
    TEST_ASSERT_EQUAL(1L, clampFrequency(-100));
    TEST_ASSERT_EQUAL(1L, clampFrequency(0));
}

void test_clamp_frequency_valid(void) {
    TEST_ASSERT_EQUAL(1000L, clampFrequency(1000));
    TEST_ASSERT_EQUAL(5000000L, clampFrequency(5000000));
}

void test_clamp_frequency_overflow(void) {
    TEST_ASSERT_EQUAL(12500000L, clampFrequency(20000000));
    TEST_ASSERT_EQUAL(12500000L, clampFrequency(99999999));
}

// Test step value cycling
void test_next_step_value_progression(void) {
    TEST_ASSERT_EQUAL(10L, nextStepValue(1));
    TEST_ASSERT_EQUAL(100L, nextStepValue(10));
    TEST_ASSERT_EQUAL(1000L, nextStepValue(100));
    TEST_ASSERT_EQUAL(10000L, nextStepValue(1000));
    TEST_ASSERT_EQUAL(100000L, nextStepValue(10000));
    TEST_ASSERT_EQUAL(1000000L, nextStepValue(100000));
}

void test_next_step_value_wraps(void) {
    TEST_ASSERT_EQUAL(1L, nextStepValue(1000000));
}

// Test boundary conditions
void test_max_frequency_constant(void) {
    TEST_ASSERT_EQUAL(12500000L, MaxAD9833Freq);
}

void test_max_step_constant(void) {
    TEST_ASSERT_EQUAL(1000000L, MaxStep);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Frequency scale tests
    RUN_TEST(test_frequency_scale_hz);
    RUN_TEST(test_frequency_scale_khz);
    RUN_TEST(test_frequency_scale_mhz);

    // Divisor tests
    RUN_TEST(test_frequency_divisor);

    // Clamping tests
    RUN_TEST(test_clamp_frequency_negative);
    RUN_TEST(test_clamp_frequency_valid);
    RUN_TEST(test_clamp_frequency_overflow);

    // Step progression tests
    RUN_TEST(test_next_step_value_progression);
    RUN_TEST(test_next_step_value_wraps);

    // Constant tests
    RUN_TEST(test_max_frequency_constant);
    RUN_TEST(test_max_step_constant);

    return UNITY_END();
}
