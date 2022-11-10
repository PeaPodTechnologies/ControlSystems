#ifndef CSOS_INTERFACES_PWM_H_
#define CSOS_INTERFACES_PWM_H_

#include <Arduino.h>
#include <I2CIP.h>

#include <types.h>
#include <interface.h>

// MACROS
// Registers
#define PCA9685_MODE1         0x00 // Mode Register 1
#define PCA9685_MODE2         0x01 // Mode Register 2
#define PCA9685_SUBADR1       0x02 // I2C-bus subaddress 1
#define PCA9685_SUBADR2       0x03 // I2C-bus subaddress 2
#define PCA9685_SUBADR3       0x04 // I2C-bus subaddress 3
#define PCA9685_ALLCALLADR    0x05 // LED All Call I2C-bus address
#define PCA9685_PRESCALE      0xFE // Prescaler for PWM output frequency
#define PCA9685_TESTMODE      0xFF // defines the test mode to be entered

// Bits - Mode 1 Register
#define MODE1_ALLCAL  0x01 // respond to LED All Call I2C-bus address
#define MODE1_SUB3    0x02 // respond to I2C-bus subaddress 3
#define MODE1_SUB2    0x04 // respond to I2C-bus subaddress 2
#define MODE1_SUB1    0x08 // respond to I2C-bus subaddress 1
#define MODE1_SLEEP   0x10 // Low power mode. Oscillator off
#define MODE1_AI      0x20 // Auto-Increment enabled
#define MODE1_EXTCLK  0x40 // Use EXTCLK pin clock
#define MODE1_RESTART 0x80 // Restart enabled

// Bits - Mode 2 Register
#define MODE2_OUTNE_0 0x01 // Active LOW output enable input
#define MODE2_OUTNE_1 0x02 // Active LOW output enable input - high impedence
#define MODE2_OUTDRV  0x04 // totem pole structure vs open-drain
#define MODE2_OCH     0x08 // Outputs change on ACK vs STOP
#define MODE2_INVRT   0x10 // Output logic state inverted

// Settings
#define PWM_ADDR  0x40
#define PWM_FREQ  490
#define PWM_ID   "pwm"

#define PWM_CHANNEL_TO_LEDREG(channel) (uint8_t)(0x06 + (4 * channel))

// Interface class for the PCA9685 16-channel 12-bit PWM IC
class PWM : public OutputInterface<uint16_t, pwm_channel_t> {
  // Note: unsigned 16-bit args are TRUNCATED to 12-bit PWM control
  private:

  public:
    typedef enum {
      PWM_CHANNEL_0,
      PWM_CHANNEL_1,
      PWM_CHANNEL_2,
      PWM_CHANNEL_3,
      PWM_CHANNEL_4,
      PWM_CHANNEL_5,
      PWM_CHANNEL_6,
      PWM_CHANNEL_7,
      PWM_CHANNEL_8,
      PWM_CHANNEL_9,
      PWM_CHANNEL_10,
      PWM_CHANNEL_11,
      PWM_CHANNEL_12,
      PWM_CHANNEL_13,
      PWM_CHANNEL_14,
      PWM_CHANNEL_15,
    } pwm_channel_t;
    static i2cip_errorlevel_t set(const i2cip_fqa_t& fqa, const uint16_t& value, const pwm_channel_t& args) override;
    static const char* getID(void) override;
};

#endif