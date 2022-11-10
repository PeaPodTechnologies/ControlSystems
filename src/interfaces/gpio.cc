#include <interfaces/gpio.h>

#include <Arduino.h>
#include <I2CIP.h>

#include <types.h>
#include <interface.h>

using namespace I2CIP;

static i2cip_errorlevel_t GPIO::get(const i2cip_fqa_t& fqa, gpio_pinstate_t& dest, const gpio_pin_t& args) override {
  // Set pin mode
  i2cip_errorlevel_t errlev = GPIO::pinMode(fqa, pin, GPIO_PINMODE_INPUT);
  I2CIP_ERR_BREAK(errlev);

  // Read in register
  uint8_t value;
  errlev = I2CIP::Device::readRegisterByte(fqa, GPIO_PIN_REG(GPIO_GPIO, pin), value);
  dest = (errlev > I2CIP_ERR_NONE ? GPIO_PIN_UNDEF : CSOS_READ_BITS(value, GPIO_PIN_SHIFT(pin), 1));
  return errlev;
}

static i2cip_errorlevel_t GPIO::set(const i2cip_fqa_t& fqa, const gpio_pinstate_t& value, const gpio_pin_t& args) override {
  if(value == GPIO_PIN_UNDEF) {
    return I2CIP_ERR_SOFT;
  }

  // Set pin mode
  i2cip_errorlevel_t errlev = GPIO::pinMode(fqa, pin, GPIO_PINMODE_OUTPUT);
  I2CIP_ERR_BREAK(errlev);
  
  // Read register
  uint8_t existing;
  errlev = I2CIP::Device::readRegisterByte(fqa, GPIO_PIN_REG(GPIO_GPIO, pin), existing);
  I2CIP_ERR_BREAK(errlev);

  // Write register
  return I2CIP::Device::writeRegister(fqa, GPIO_PIN_REG(GPIO_GPIO, pin), CSOS_OVERWRITE_BITS(existing, value, (pin % 8), 1));
}

i2cip_errorlevel_t GPIO::pinMode(const i2cip_fqa_t& fqa, const gpio_pin_t& pin, const gpio_pinmode_t& mode) {
  // Read in register
  uint8_t existing;
  i2cip_errorlevel_t errlev = I2CIP::Device::readRegisterByte(fqa, GPIO_PIN_REG(GPIO_IODIR, pin), existing);
  I2CIP_ERR_BREAK(errlev);

  // Write to register
  return I2CIP::Device::writeRegister(fqa, GPIO_PIN_REG(GPIO_IODIR, pin), CSOS_OVERWRITE_BITS(existing, mode, (pin % 8), 1));
}

static const char* getID(void) {
  return GPIO_ID;
}