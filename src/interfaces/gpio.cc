#include <interfaces/gpio.h>

#include <Adafruit_BusIO_Register.h>

#include <types.h>
#include <interface.h>
#include <interfaces/mux.h>

errorlevel_t Interface::GPIO::pinMode(uint8_t pin, uint8_t mode) {
  // Get registers of interest: IO direction and pullup configuration
  Adafruit_BusIO_Register iodir(&thisdevice, GPIO_REG(GPIO_IODIR, GPIO_PORT(pin)));
  Adafruit_BusIO_Register pullup_config(&thisdevice, GPIO_REG(GPIO_GPPU, GPIO_PORT(pin)));

  // Get bits of interest for this specific pin
  Adafruit_BusIO_RegisterBits iodir_bit(&iodir, 1, pin % 8);
  Adafruit_BusIO_RegisterBits pullup_bit(&pullup_config, 1, pin % 8);

  // Write to those registers
  errorlevel_t result = mux->setBus(bus);
  result &= (iodir_bit.write((mode == OUTPUT) ? 0 : 1) ? ERR_NONE : ERR_SOFT);
  result &= (pullup_bit.write((mode == INPUT_PULLUP) ? 1 : 0) ? ERR_NONE : ERR_SOFT);
  result &= mux->resetBus();
  return ERR_SOFT;
}

errorlevel_t Interface::GPIO::readPort(gpioport_t port, uint8_t* dest) {
  Adafruit_BusIO_Register gpio(&thisdevice, GPIO_REG(GPIO_GPIO, port));
  errorlevel_t result = mux->setBus(bus);
  uint32_t val = gpio.read();
  result &= mux->resetBus();
  if (val == 0xFFFFFFFF) {
    // Failed read
    result &= ERR_SOFT;
  } else {
    // Truncate to 8 LSB; store
    *dest = (uint8_t)(val & 0xFF);
  }
  return result;
}

errorlevel_t Interface::GPIO::digitalRead(uint8_t pin, uint8_t* dest) {
  uint8_t result;
  errorlevel_t status = readPort(GPIO_PORT(pin), &result);
  if (status == ERR_NONE) {
    // Shift right to target the specified pin, and extract LSB
    *dest = (((result >> (pin % 8)) & 0x1) == 0 ? LOW : HIGH);
  }
  return status;
}

errorlevel_t Interface::GPIO::digitalWrite(uint8_t pin, uint8_t value) {
  errorlevel_t result = mux->setBus(bus);
  Adafruit_BusIO_Register gpio(&thisdevice, GPIO_REG(GPIO_GPIO, GPIO_PORT(pin)));
  Adafruit_BusIO_RegisterBits pin_bit(&gpio, 1, pin % 8);

  result &= (pin_bit.write((value == LOW) ? 0 : 1) ? ERR_NONE : ERR_SOFT);
  result &= mux->resetBus();
  return result;
}