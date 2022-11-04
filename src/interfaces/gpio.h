#ifndef CSOS_INTERFACES_GPIO_H_
#define CSOS_INTERFACES_GPIO_H_

#include <Arduino.h>

#include <types.h>
#include <interface.h>

// MACROS
// Registers
#define GPIO_IODIR    0x00 // I/O direction register
#define GPIO_IPOL     0x02 // Input polarity register
#define GPIO_GPINTEN  0x04 // Interrupt-on-change control register
#define GPIO_DEFVAL   0x06 // Default compare register for interrupt-on-change
#define GPIO_INTCON   0x08 // Interrupt control register
#define GPIO_IOCON    0x0A // Configuration register
#define GPIO_GPPU     0x0C // Pull-up resistor configuration register
#define GPIO_INTF     0x0E // Interrupt flag register
#define GPIO_INTCAP   0x10 // Interrupt capture register
#define GPIO_GPIO     0x12 // Port register
#define GPIO_OLAT     0x14 // Output latch register

// Settings
#define GPIO_ADDR     0x20 // Default address

#define GPIO_PORT(pin) (gpioport_t)((pin < 8) ? 0 : 1)          // Determine port from pin number
#define GPIO_REG(reg, port) (uint16_t)(port == 0 ? reg : reg+1) // Determine register from base address and port

// Interface class for the MCP23017 I2C GPIO expander IC
namespace Interface::GPIO {

  typedef enum gpioport_t {
    PORT_A,
    PORT_B
  } gpioport_t;
  
  errorlevel_t pinMode(uint8_t pin, uint8_t mode);
  errorlevel_t readPort(gpioport_t port, uint8_t* dest);
  errorlevel_t digitalWrite(uint8_t pin, uint8_t value);
  errorlevel_t digitalRead(uint8_t pin, uint8_t* dest);
};

#endif