#ifndef CSOS_INTERFACES_ADC_H_
#define CSOS_INTERFACES_ADC_H_

#include <Arduino.h>

#include <types.h>
#include <interface.h>

// Not sure if this will break something. Let's try it and find out
#undef ADC

// REGISTERS

// Pointer register
#define ADC_REG_POINTER_MASK        0x03 // Point mask
#define ADC_REG_POINTER_CONVERT     0x00 // Conversion
#define ADC_REG_POINTER_CONFIG      0x01 // Configuration
#define ADC_REG_POINTER_LOWTHRESH   0x02 // Low threshold
#define ADC_REG_POINTER_HITHRESH    0x03 // High threshold

// Config register: OS bit
#define ADC_REG_CONFIG_OS_MASK      0x8000 // OS Mask
#define ADC_REG_CONFIG_OS_SINGLE    0x8000 // Write: Set to start a single-conversion
#define ADC_REG_CONFIG_OS_BUSY      0x0000 // Read: Bit = 0 when conversion is in progress
#define ADC_REG_CONFIG_OS_NOTBUSY   0x8000 // Read: Bit = 1 when device is not performing a conversion

// Config register: MUX bits
#define ADC_REG_CONFIG_MUX_MASK     0x7000 // Mux Mask
#define ADC_REG_CONFIG_MUX_DIFF_0_1 0x0000 // Differential P = AIN0, N = AIN1 (default)
#define ADC_REG_CONFIG_MUX_DIFF_0_3 0x1000 // Differential P = AIN0, N = AIN3
#define ADC_REG_CONFIG_MUX_DIFF_1_3 0x2000 // Differential P = AIN1, N = AIN3
#define ADC_REG_CONFIG_MUX_DIFF_2_3 0x3000 // Differential P = AIN2, N = AIN3
#define ADC_REG_CONFIG_MUX_SINGLE_0 0x4000 // Single-ended AIN0
#define ADC_REG_CONFIG_MUX_SINGLE_1 0x5000 // Single-ended AIN1
#define ADC_REG_CONFIG_MUX_SINGLE_2 0x6000 // Single-ended AIN2
#define ADC_REG_CONFIG_MUX_SINGLE_3 0x7000 // Single-ended AIN3

// Config register: Gain bits
#define ADC_REG_CONFIG_PGA_MASK     0x0E00 // PGA Mask
#define ADC_REG_CONFIG_PGA_6_144V   0x0000 // +/-6.144V range = Gain 2/3
#define ADC_REG_CONFIG_PGA_4_096V   0x0200 // +/-4.096V range = Gain 1
#define ADC_REG_CONFIG_PGA_2_048V   0x0400 // +/-2.048V range = Gain 2 (default)
#define ADC_REG_CONFIG_PGA_1_024V   0x0600 // +/-1.024V range = Gain 4
#define ADC_REG_CONFIG_PGA_0_512V   0x0800 // +/-0.512V range = Gain 8
#define ADC_REG_CONFIG_PGA_0_256V   0x0A00 // +/-0.256V range = Gain 16

// Config register: ADC mode bit
#define ADC_REG_CONFIG_MODE_MASK    0x0100 // Mode Mask
#define ADC_REG_CONFIG_MODE_CONTIN  0x0000 // Continuous conversion mode
#define ADC_REG_CONFIG_MODE_SINGLE  0x0100 // Power-down single-shot mode (default)

// Config register: sample rate bits
#define ADC_REG_CONFIG_RATE_MASK    0x00E0 // Data Rate Mask
#define ADC_RATE_128SPS             0x0000 // 128 samples per second
#define ADC_RATE_250SPS             0x0020 // 250 samples per second
#define ADC_RATE_490SPS             0x0040 // 490 samples per second
#define ADC_RATE_920SPS             0x0060 // 920 samples per second
#define ADC_RATE_1600SPS            0x0080 // 1600 samples per second (default)
#define ADC_RATE_2400SPS            0x00A0 // 2400 samples per second
#define ADC_RATE_3300SPS            0x00C0 // 3300 samples per second

// Config register: Comparator mode bit
#define ADC_REG_CONFIG_CMODE_MASK   0x0010 // CMode Mask
#define ADC_REG_CONFIG_CMODE_TRAD   0x0000 // Traditional comparator with hysteresis (default)
#define ADC_REG_CONFIG_CMODE_WINDOW 0x0010 // Window comparator

// Config register: Comparator polarity bit
#define ADC_REG_CONFIG_CPOL_MASK    0x0008 // CPol Mask
#define ADC_REG_CONFIG_CPOL_ACTVLOW 0x0000 // ALERT/RDY pin is low when active (default)
#define ADC_REG_CONFIG_CPOL_ACTVHI  0x0008 // ALERT/RDY pin is high when active

// Config register: Comparator latching bit
#define ADC_REG_CONFIG_CLAT_MASK    0x0004 // Determines if ALERT/RDY pin latches once asserted
#define ADC_REG_CONFIG_CLAT_NONLAT  0x0000 // Non-latching comparator (default)
#define ADC_REG_CONFIG_CLAT_LATCH   0x0004 // Latching comparator

// Config register: Comparator queue bits
#define ADC_REG_CONFIG_CQUE_MASK    0x0003 // CQue Mask
#define ADC_REG_CONFIG_CQUE_1CONV   0x0000 // Assert ALERT/RDY after one conversions
#define ADC_REG_CONFIG_CQUE_2CONV   0x0001 // Assert ALERT/RDY after two conversions
#define ADC_REG_CONFIG_CQUE_4CONV   0x0002 // Assert ALERT/RDY after four conversions
#define ADC_REG_CONFIG_CQUE_NONE    0x0003 // Disable the comparator and put ALERT/RDY in high state (default)

// HELPERS
#define ADC_CHANNEL_TO_MUX(channel) (0x4000 + (channel) * 0x1000)

// SETTINGS
#define ADC_ADDR  0x48
#define ADC_SHIFT 4                   // Bit shift (12-bit ADC -> int16_t)
#define ADC_GAIN  ADC::GAIN_TWOTHIRDS // +/-6.144V for 0-5V single-ended analogRead
#define ADC_SPS   ADC_RATE_1600SPS

// Interface class for the ADS1015 12-bit ADC IC
namespace Interface::ADC {
  // Gain settings
  typedef enum {
    GAIN_TWOTHIRDS = ADC_REG_CONFIG_PGA_6_144V,
    GAIN_ONE = ADC_REG_CONFIG_PGA_4_096V,
    GAIN_TWO = ADC_REG_CONFIG_PGA_2_048V,
    GAIN_FOUR = ADC_REG_CONFIG_PGA_1_024V,
    GAIN_EIGHT = ADC_REG_CONFIG_PGA_0_512V,
    GAIN_SIXTEEN = ADC_REG_CONFIG_PGA_0_256V
  } adc_gain_t;

  bool ready(void);

  float computeVolts(int16_t counts);
  errorlevel_t readVoltage(uint8_t channel, float* dest);
};

#endif