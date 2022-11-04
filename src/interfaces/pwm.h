#ifndef CSOS_INTERFACES_PWM_H_
#define CSOS_INTERFACES_PWM_H_

#include <Arduino.h>

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
#define MODE2_OUTNE_1 0x02 // Active LOW output enable input - high impedience
#define MODE2_OUTDRV  0x04 // totem pole structure vs open-drain
#define MODE2_OCH     0x08 // Outputs change on ACK vs STOP
#define MODE2_INVRT   0x10 // Output logic state inverted

// Constants
#define PWM_OSC_FREQ      25000000
#define PWM_FREQ_MIN      1
#define PWM_FREQ_MAX      3500
#define PWM_PRESCALE_MIN  3
#define PWM_PRESCALE_MAX  255
#define PWM_EPSILON       0.005

// Settings
#define PWM_ADDR  0x40
#define PWM_FREQ  1000

// Interface class for the PCA9685 PWM IC
namespace Interface::PWM {
  errorlevel_t setPWMFreq(float freq);
  errorlevel_t begin(void);
  errorlevel_t setPWM(uint8_t pin, float pwm);
  errorlevel_t reset(void);
};

#endif